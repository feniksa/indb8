/* @@@LICENSE
*
* Copyright (c) 2013 LG Electronics, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */

#include "db/MojDbShardEngine.h"
#include "db/MojDbKind.h"
#include "db/MojDb.h"
#include "db/MojDbServiceDefs.h"
#include "db/MojDbMediaLinkManager.h"
#include "core/MojDataSerialization.h"
#include "core/MojLog.h"
#include <boost/crc.hpp>
#include <string>

using namespace std;



static const MojChar* const ShardInfoKind1Str =
    _T("{\"id\":\"ShardInfo1:1\",")
    _T("\"owner\":\"mojodb.admin\",")
    _T("\"indexes\":[ {\"name\":\"ShardId\",   \"props\":[ {\"name\":\"shardId\"} ]}, \
                      {\"name\":\"DeviceId\",  \"props\":[ {\"name\":\"deviceId\"} ]}, \
                      {\"name\":\"IdBase64\",  \"props\":[ {\"name\":\"idBase64\"} ]}, \
                      {\"name\":\"Active\",    \"props\":[ {\"name\":\"active\"} ]}, \
                      {\"name\":\"Transient\", \"props\":[ {\"name\":\"transient\"} ]}, \
                      {\"name\":\"Timestamp\", \"props\":[ {\"name\":\"timestamp\"} ]}, \
                      {\"name\":\"KindIds\", \"props\":[ {\"name\":\"kindIds\"} ]}\
                    ]}");

MojLogger MojDbShardEngine::s_log(_T("db.shardEngine"));

MojDbShardEngine::MojDbShardEngine(MojDb& db)
  : m_db(db)
{
    MojLogTrace(s_log);
}

MojDbShardEngine::~MojDbShardEngine(void)
{
    MojLogTrace(s_log);
}

/**
 * initialize MojDbShardEngine
 *
 * @param ip_db
 *   pointer to MojDb instance
 *
 * @param io_req
 *   batch support
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::init (const MojObject& conf, MojDbReqRef req)
{
    MojLogTrace(s_log);
    MojErr err;
    MojObject obj;

    err = configure(conf);
    MojErrCheck(err);

    // add type
    err = obj.fromJson(ShardInfoKind1Str);
    MojErrCheck(err);
    err = m_db.kindEngine()->putKind(obj, req, true); // add builtin kind
    MojErrCheck(err);

    //all devices should not be active at startup
    err = resetShards(req);
    MojErrCheck(err);

    err = initCache(req);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbShardEngine::configure(const MojObject& conf)
{
    MojLogTrace(s_log);

    MojErr err;

    MojObject mediaConf;
    MojString mediaLinkDirectory;

    if (conf.get(_T("mediaMountpointDirectory"), mediaConf)) {
        err = mediaConf.stringValue(mediaLinkDirectory);
        MojErrCheck(err);
        MojLogDebug(s_log, "Use mediaMountpointDirectory as \"%s\"", mediaLinkDirectory.data());
    } else {
        MojLogDebug(s_log, "Configuration section \"mediaMountpointDirectory\" doesn't found, use default value for mediaMountpointDirectory");
    }

    err = m_mediaLinkManager.setLinkDirectory(mediaLinkDirectory);
    MojErrCheck(err);

    return MojErrNone;
}

/**
 * all devices should not be active at startup:
 * - reset 'active flag'
 * - reset 'mountPath'
 *
 * @param io_req
 *   batch support
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::resetShards (MojDbReq& io_req)
{
    MojDbQuery query;
    MojDbCursor cursor;
    MojObject props;
    MojUInt32 count;

    MojErr err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);

    err = props.put(_T("active"), false);
    MojErrCheck(err);
    err = props.put(_T("mountPath"), MojString());
    MojErrCheck(err);

    err = m_db.merge(query, props, count, MojDb::FlagNone, io_req);
    MojErrCheck(err);

    return MojErrNone;
}

/**
 * put a new shard description to db
 *
 * @param shardInfo
 *   device info to store
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::put (const MojDbShardInfo& shardInfo, MojDbReqRef req)
{
    MojLogTrace(s_log);
    MojAssert(shardInfo.id);

    MojObject obj;
    MojErr err;

    err = obj.putString(_T("_kind"), _T("ShardInfo1:1"));
    MojErrCheck(err);

    MojDbShardInfo info = shardInfo;
    updateTimestamp(info);

    if (info.id_base64.empty())
    {
        err = MojDbShardEngine::convertId(info.id, info.id_base64);
        MojErrCheck(err);
    }

    err = convert(info, obj);
    MojErrCheck(err);
    err = m_db.put(obj, MojDb::FlagNone, req);
    MojErrCheck(err);

    m_cache.put(shardInfo.id, obj);

    return MojErrNone;
}

/**
 * get shard description by id
 *
 * @param shardId
 *   device id
 *
 * @param shardInfo
 *   device info, initialized if device found by id
 *
 * @param found
 *   true if found
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::get (MojUInt32 shardId, MojDbShardInfo& shardInfo, bool& found)
{
    MojErr err;
    MojAssert(mp_db);
    MojObject dbObj;

    found = m_cache.get(shardId, dbObj);

    if (found)
    {
        err = convert(dbObj, shardInfo);
        MojErrCheck(err);
    }

    return MojErrNone;
}

/**
 * get list of all active shards
 *
 * @param shardInfoList
 *   list of device info collect all shards with state 'active'==true
 *
 * @param count
 *   number of devices info added
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::getAllActive (std::list<MojDbShardInfo>& shardInfoList, MojUInt32& count, MojDbReqRef req)
{
    MojLogTrace(s_log);
    MojAssert(mp_db);

    MojErr err;

    MojDbQuery query;
    MojDbCursor cursor;
    MojObject obj(true);
    MojDbShardInfo shardInfo;

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query.where(_T("active"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    err = m_db.find(query, cursor, req);
    MojErrCheck(err);

    count = 0;
    shardInfoList.clear();

    while (true)
    {
        bool found;
        MojObject dbObj;

        err = cursor.get(dbObj, found);
        MojErrCheck(err);
        if (!found)
            break;

        err = convert(dbObj, shardInfo);
        MojErrCheck(err);

        shardInfoList.push_back(shardInfo);
        ++count;
    }

    return MojErrNone;
}

/**
 * update shardInfo
 *
 * @param i_shardInfo
 *   update device info properties (search by ShardInfo.id)
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::update (const MojDbShardInfo& i_shardInfo, MojDbReqRef req)
{
    MojLogTrace(s_log);
    MojErr err;

    MojDbQuery query;
    MojObject dbObj;
    MojObject obj(i_shardInfo.id);

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query.where(_T("shardId"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    MojObject update;
    MojUInt32 count = 0;

    MojDbShardInfo shardInfo = i_shardInfo;
    updateTimestamp(shardInfo);

    err = convert(shardInfo, update);
    MojErrCheck(err);

    err = m_db.merge(query, update, count, MojDb::FlagNone, req);
    MojErrCheck(err);

    if (count == 0)
        return MojErrDbObjectNotFound;

    m_cache.update(i_shardInfo.id, update);

    return MojErrNone;
}

/**
 * get shard description by uuid
 *
 * @param deviceUuid
 *   uuid
 *
 * @param shardInfo
 *   device info (initialized if found)
 *
 * @param found
 *   true if found
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::getByDeviceUuid (const MojString& deviceUuid, MojDbShardInfo& shardInfo, bool& found)
{
    MojLogTrace(s_log);
    MojErr err;

    //get record from db, extract id
    MojDbQuery query;
    MojDbCursor cursor;
    MojObject obj(deviceUuid);
    MojObject dbObj;

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query.where(_T("deviceId"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    err = m_db.find(query, cursor);
    MojErrCheck(err);

    err = cursor.get(dbObj, found);
    MojErrCheck(err);

    if (found)
        convert(dbObj, shardInfo);

    return MojErrNone;
}

/**
 * get device id by uuid
 *
 * search within db for i_deviceId, return id if found
 * else
 * allocate a new id
 *
 * @param deviceUuid
 *   device uuid
 *
 * @param shardId
 *   device id
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::getShardId (const MojString& deviceUuid, MojUInt32& shardId)
{
    MojLogTrace(s_log);
    MojErr err;
    MojDbShardInfo shardInfo;
    bool found;

    err = getByDeviceUuid(deviceUuid, shardInfo, found);
    MojErrCheck(err);
    if (found)  {
        shardId = shardInfo.id;
        MojLogDebug(s_log, _T("Shard id for device %s is %d"), deviceUuid.data(), shardId);
    } else {
        MojLogDebug(s_log, _T("Shard id for device %s not found, generating it"), deviceUuid.data());
        err = allocateId(deviceUuid, shardId);
        MojErrCheck(err);
    }

    return MojErrNone;
}

/**
 * compute a new shard id
 *
 * @param deviceUuid
 *   device uuid
 *
 * @param shardId
 *   device id
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::allocateId (const MojString& deviceUuid, MojUInt32& shardId)
{
    MojLogTrace(s_log);

    MojErr err;
    MojUInt32 id;
    MojUInt32 calc_id;
    MojUInt32 prefix = 1;
    MojUInt32 suffix = 1;
    bool found = false;

    err = computeId(deviceUuid, calc_id);
    MojErrCheck(err);

    do
    {
        id = calc_id | (prefix * 0x01000000);

        //check the table to see if this ID already exists
        err = isIdExist(id, found);
        MojErrCheck(err);

        if (found) {
            MojLogWarning(MojDbShardEngine::s_log, _T("id generation -> [%x] exist already, prefix = %u\n"), id, prefix);
            prefix++;
        } else {
            MojAssert(id);

            shardId = id;
            break;  // exit from loop
        }

        if (prefix == 128)
        {
            MojLogWarning(MojDbShardEngine::s_log, _T("id generation -> next iteration\n"));
            prefix = 1;
            MojString modified_uuid;
            modified_uuid.format("%s%x", deviceUuid.data(), ++suffix);
            computeId(modified_uuid, calc_id); //next iteration
        }
    }
    while (!found);

    return MojErrNone;
}

/**
 * is device id exist?
 *
 * @param shardId
 *   device id
 *
 * @param found
 *   true, if found
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::isIdExist (MojUInt32 shardId, bool& found)
{
    found = m_cache.isExist(shardId);
    return MojErrNone;
}

/**
 * compute device id by media uuid
 *
 * @param mediaUuid
 *   media uuid
 *
 * @param shardId
 *   device id
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::computeId (const MojString& mediaUuid, MojUInt32& sharId)
{
    MojAssert(!mediaUuid.empty());

    std::string str = mediaUuid.data();

    //Create a 24 bit hash of the string
    boost::crc_32_type result;
    result.process_bytes(str.data(), str.length());
    MojInt32 code = result.checksum();
    result.reset();

    //Prefix the 24 bit hash with 0x01 to create a 32 bit unique shard ID
    sharId = code & 0xFFFFFF;

    return MojErrNone;
}

/**
 * convert device id to base64 string
 *
 * @param i_id
 *   device id
 *
 * @param o_id_base64
 *   device id converted to base64 string
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::convertId (const MojUInt32 i_id, MojString& o_id_base64)
{
    MojErr err;
    MojBuffer buf;
    MojDataWriter writer(buf);

    err = writer.writeUInt32(i_id);
    MojErrCheck(err);

    MojVector<MojByte> byteVec;
    err = buf.toByteVec(byteVec);
    MojErrCheck(err);
    MojString str;
    err = o_id_base64.base64Encode(byteVec, false);
    MojErrCheck(err);

    return MojErrNone;
}

/**
 * convert base64 string to device id
 *
 * @param i_id_base64
 *   device id converted to base64 string
 *
 * @param o_id
 *   device id
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::convertId (const MojString& i_id_base64, MojUInt32& o_id)
{
    MojErr err;
    MojVector<MojByte> idVec;
    err = i_id_base64.base64Decode(idVec);
    MojErrCheck(err);

    // extract first 32bits of _id as shard id in native order
    MojDataReader reader(idVec.begin(), idVec.size());

    err = reader.readUInt32(o_id);
    MojErrCheck(err);

    return MojErrNone;
}

/**
 * convert device info to MojObject
 *
 * @param i_shardInfo
 *   device info
 *
 * @param o_obj
 *   MojObject
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::convert (const MojDbShardInfo& i_shardInfo, MojObject& o_obj)
{
    MojObject obj1(i_shardInfo.id);
    MojErr err = o_obj.put(_T("shardId"), obj1);
    MojErrCheck(err);

    MojObject obj2(i_shardInfo.active);
    err = o_obj.put(_T("active"), obj2);
    MojErrCheck(err);

    err = o_obj.put(_T("transient"), i_shardInfo.transient);
    MojErrCheck(err);

    MojObject obj3(i_shardInfo.id_base64);
    err = o_obj.put(_T("idBase64"), obj3);
    MojErrCheck(err);

    MojObject obj4(i_shardInfo.deviceId);
    err = o_obj.put(_T("deviceId"), obj4);
    MojErrCheck(err);

    MojObject obj5(i_shardInfo.deviceUri);
    err = o_obj.put(_T("deviceUri"), obj5);
    MojErrCheck(err);

    MojObject obj6(i_shardInfo.mountPath);
    err = o_obj.put(_T("mountPath"), obj6);
    MojErrCheck(err);

    MojObject obj7(i_shardInfo.deviceName);
    err = o_obj.put(_T("deviceName"), obj7);
    MojErrCheck(err);

    MojObject obj8(i_shardInfo.timestamp);
    err = o_obj.put(_T("timestamp"), obj8);
    MojErrCheck(err);

    //convert kindIds
    MojString strKindIds;
    err = i_shardInfo.kindIds.toString(strKindIds);
    MojErrCheck(err);
    MojObject obj9(strKindIds);
    err = o_obj.put(_T("kindIds"), obj9);
    MojErrCheck(err);

    return MojErrNone;
}

/**
 * convert MojObject to device info
 *
 * @param i_obj
 *   MojObject, input
 *
 * @param o_shardInfo
 *   device info
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::convert (const MojObject& i_obj, MojDbShardInfo& o_shardInfo)
{
    MojErr err = i_obj.getRequired(_T("shardId"), o_shardInfo.id);
    MojErrCheck(err);

    err = i_obj.getRequired(_T("idBase64"), o_shardInfo.id_base64);
    MojErrCheck(err);

    err = i_obj.getRequired(_T("active"), o_shardInfo.active);
    MojErrCheck(err);

    err = i_obj.getRequired(_T("transient"), o_shardInfo.transient);
    MojErrCheck(err);

    err = i_obj.getRequired(_T("deviceId"), o_shardInfo.deviceId);
    MojErrCheck(err);

    err = i_obj.getRequired(_T("deviceUri"), o_shardInfo.deviceUri);
    MojErrCheck(err);

    err = i_obj.getRequired(_T("mountPath"), o_shardInfo.mountPath);
    MojErrCheck(err);

    err = i_obj.getRequired(_T("deviceName"), o_shardInfo.deviceName);
    MojErrCheck(err);

    err = i_obj.getRequired(_T("timestamp"), o_shardInfo.timestamp);
    MojErrCheck(err);

    MojString strKindIds;
    err = i_obj.getRequired(_T("kindIds"), strKindIds);
    MojErrCheck(err);
    err = o_shardInfo.kindIds.fromString(strKindIds);
    MojErrCheck(err);

    return MojErrNone;
}

/**
 * Support garbage collection of obsolete shards
 * remove shard objects older <numDays> days
 *
 * @param numDays
 *   days
 *
 * @param req
 *   batch support
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::purgeShardObjects (MojInt64 numDays, MojDbReqRef req)
{
    MojDbQuery query1, query2;
    MojDbCursor cursor;
    MojInt32 value_id = 0;
    MojInt64 value_timestamp;
    MojObject obj(value_id);
    MojObject dbObj;
    MojObject obj_active(false);
    MojVector<MojUInt32> arrShardIds;
    MojString shardIdStr;
    bool found;
    bool value_active;

    MojTime time;
    MojErrCheck( MojGetCurrentTime(time) );
    MojInt64 purgeTime = time.microsecs() - (MojTime::UnitsPerDay * numDays);
    MojLogDebug(s_log, _T("purging objects for shards inactive for more than %lld days..."), numDays);

    //collect 'old' shards
    //--------------------
    MojErr err = query1.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query1.where(_T("timestamp"), MojDbQuery::OpLessThanEq, purgeTime);
    MojErrCheck(err);
    query1.setIgnoreInactiveShards(false);

    err = m_db.find(query1, cursor, req);
    MojErrCheck(err);

    while (true)
    {
        err = cursor.get(dbObj, found);
        MojErrCheck(err);
        if(!found)
            break;

        err = dbObj.getRequired(_T("shardId"), value_id);
        MojErrCheck(err);
        err = MojDbShardEngine::convertId(value_id, shardIdStr);
        MojErrCheck(err);
        err = dbObj.getRequired(_T("active"), value_active);
        MojErrCheck(err);

        if(!value_active)
        {
            err = arrShardIds.pushUnique(value_id);
            MojErrCheck(err);
            MojLogDebug(s_log, _T("Need to purge records for old shard: [%s]"), shardIdStr.data());
        } else { // TODO: Remove
            MojLogDebug(s_log, _T("Ignore active shard: [%s]"), shardIdStr.data());
        }

    }

    MojErrCheck(cursor.close());

    removeShardObjects(arrShardIds, req);
    MojLogDebug(s_log, _T("Ended"));

    return MojErrNone;
}

/**
 * removeShardObjects
 *
 * @param strShardIdToRemove
 *   device id
 *
 * @param req
 *   batch support
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::removeShardObjects (const MojString& strShardIdToRemove, MojDbReqRef req)
{
    MojUInt32 shardId;

    MojVector<MojUInt32> shardIds;
    MojErr err = MojDbShardEngine::convertId(strShardIdToRemove, shardId);
    MojErrCheck(err);
    err = shardIds.push(shardId);
    MojErrCheck(err);
    MojLogDebug(s_log, _T("purging objects for shard: %s"), strShardIdToRemove.data());


    return(removeShardObjects(shardIds, req));
}

/**
 * removeShardObjects
 *
 * @param shardId
 *   device id
 *
 * @param req
 *   batch support
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::removeShardObjects (const MojVector<MojUInt32>& arrShardIds, MojDbReqRef req)
{
    MojVector<MojObject> objList;
    MojDbShardInfo info;
    bool foundOut;
    MojErr err;
    MojDbKind* pKind;

    if (arrShardIds.size() > 0)
    {
        for (MojVector<MojUInt32>::ConstIterator itShardId = arrShardIds.begin();
                itShardId != arrShardIds.end();
                ++itShardId)
            {
                //get shard info structure
                err = get(*itShardId, info, foundOut);
                MojErrCheck(err);

                if(foundOut)
                {
                    //iterate over kindIds array
                    for (std::list<MojString>::iterator itKindId = info.kindIds.begin(); itKindId != info.kindIds.end(); ++itKindId)
                    {
                        //verify kind for 'built-in' flag
                        err = m_db.kindEngine()->getKind((*itKindId).data(), pKind);
                        MojErrCheck(err);

                        if(pKind->isBuiltin())
                            continue;

                        MojLogDebug(s_log, _T("Get next shard for %s"), (*itKindId).data()); // TODO: to debug
                        err = removeShardKindObjects(*itShardId, *itKindId, req);
                        MojErrCheck(err);
                    }
                }
            }

            MojLogDebug(s_log, _T("Returned from removeShardObjects")); // TODO: to debug
    }

    return MojErrNone;
}

/**
 * removeShardRecords
 *
 * @param shardIdStr
 *   device id
 *
 * @param kindId
 *   kind id
 *
 * @param req
 *   batch support
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::removeShardKindObjects (const MojUInt32 shardId, const MojString& kindId, MojDbReq& req)
{
    // make query
    bool found;
    uint32_t countDeleted = 0;
    uint32_t countRead = 0;
    MojDbQuery query;
    MojErr err = query.from(kindId);
    MojErrCheck(err);
    query.setIgnoreInactiveShards(false);

    MojDbCursor cursor;
    err = m_db.find(query, cursor, req);
    MojErrCheck(err);

    MojString shardIdStr;
    err = MojDbShardEngine::convertId(shardId, shardIdStr);
    MojErrCheck(err);

    MojLogDebug(s_log, _T("purging objects for shard: [%s], Kind: [%s]"), shardIdStr.data(), kindId.data());// todo: convert to Info

    MojObject record;
    MojObject recordId;
    MojUInt32 cmpShardId;

    while(true)
    {
        err = cursor.get(record, found);
        MojErrCheck(err);

        if (!found)
            break;

        err = record.getRequired(MojDb::IdKey, recordId);
        MojErrCheck(err);
        countRead ++;

        err = MojDbIdGenerator::extractShard(recordId, cmpShardId);
        MojErrCheck(err);

        if (cmpShardId != shardId)
            continue;

        err = m_db.del(recordId, found, MojDb::FlagNone, req);
        MojErrCheck(err);
        countDeleted++;
    }

    if (countDeleted) {
        MojLogDebug(s_log, _T("purged %d of %d objects for shard: [%s] from Kind: [%s]"), countDeleted, countRead, shardIdStr.data(), kindId.data());
    } else {
        MojLogDebug(s_log, _T("none purged out of %d objects"), countRead); // todo: convert to Info
    }

    return MojErrNone;
}

/**
 * update ShardInfo::timestamp with current time value
 */
MojErr MojDbShardEngine::updateTimestamp (MojDbShardInfo& shardInfo)
{
    MojTime time;
    MojErrCheck( MojGetCurrentTime(time) );
    shardInfo.timestamp = time.microsecs();
    return MojErrNone;
}

/**
 * init cache
 *
 * @param io_req
 *   batch support
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::initCache (MojDbReq& io_req)
{
    MojDbQuery query;
    MojDbCursor cursor;
    MojInt32 value_id = 0;
    MojObject obj(value_id);
    MojObject dbObj;
    bool found;

    MojErr err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);

    err = m_db.find(query, cursor, io_req);
    MojErrCheck(err);

    while (true)
    {
        err = cursor.get(dbObj, found);
        MojErrCheck(err);
        if(!found)
            break;

        err = dbObj.getRequired(_T("shardId"), value_id);
        MojErrCheck(err);
        m_cache.put(value_id, dbObj);
    }

    MojErrCheck(cursor.close());

    return MojErrNone;
}

MojErr MojDbShardEngine::linkShardAndKindId (const MojString& shardIdBase64, const MojString& kindId, MojDbReqRef req)
{
    if(shardIdBase64.empty())
    {
        MojLogDebug(s_log, _T("link shard and kind: empty shardId"));
        return MojErrNone;
    }

    MojErr err;
    MojUInt32 id;
    err = convertId(shardIdBase64, id);
    MojErrCheck(err);
    err = linkShardAndKindId(id, kindId, req);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbShardEngine::linkShardAndKindId (const MojUInt32 shardId, const MojString& kindId, MojDbReqRef req)
{
    if(kindId.empty())
    {
        MojLogDebug(s_log, _T("link shard and kind: empty kindId"));
        return MojErrNone;
    }

    bool found;
    MojDbShardInfo shardInfo;
    MojErr err;

    err = get(shardId, shardInfo, found);
    MojErrCheck(err);

    if(!found)
        return MojErrNone;

    if(shardInfo.kindIds.isExist(kindId))
        return MojErrNone;

    //link shard and kindId
    shardInfo.kindIds.add(kindId);

    //update db
    err = update(shardInfo, req);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbShardEngine::unlinkShardAndKindId (const MojString& shardIdBase64, const MojString& kindId, MojDbReqRef req)
{
    if(shardIdBase64.empty())
    {
        MojLogDebug(s_log, _T("link shard and kind: empty shardId"));
        return MojErrNone;
    }

    MojErr err;
    MojUInt32 id;
    err = convertId(shardIdBase64, id);
    MojErrCheck(err);
    err = unlinkShardAndKindId(id, kindId, req);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbShardEngine::unlinkShardAndKindId (const MojUInt32 shardId, const MojString& kindId, MojDbReqRef req)
{
    if(kindId.empty())
    {
        MojLogDebug(s_log, _T("link shard and kind: empty kindId"));
        return MojErrNone;
    }

    bool found;
    MojDbShardInfo shardInfo;
    MojErr err;

    err = get(shardId, shardInfo, found);
    MojErrCheck(err);

    if(!found)
        return MojErrNone;

    if(!shardInfo.kindIds.isExist(kindId))
        return MojErrNone;

    //unlink shard and kindId
    shardInfo.kindIds.remove(kindId);

    //update db
    err = update(shardInfo, req);
    MojErrCheck(err);

    return MojErrNone;
}

/**
 * removeShardInfo record
 *
 * @param shardIdStr
 * device id
 *
 * @param kindId
 * kind id
 *
 * @param req
 * batch support
 *
 * @return MojErr
 */
MojErr MojDbShardEngine::removeShardInfo (const MojUInt32 shardId)
{
    MojUInt32 count;
    MojDbQuery query;
    MojObject obj_id(shardId);

    MojErr err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query.where(_T("shardId"), MojDbQuery::OpEq, obj_id);
    MojErrCheck(err);

    err = m_db.del(query, count, MojDb::FlagNone);
    MojErrCheck(err);

    m_cache.del(shardId);

    return MojErrNone;
}

MojErr MojDbShardEngine::processShardInfo(const MojDbShardInfo& shardInfo)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    LOG_DEBUG("[db_shardEngine] Shard engine notified about new shard");

    MojErr err;
    bool found;
    MojDbShardInfo databaseShardInfo;

    // Inside shardInfo we have only filled deviceId deviceUri mountPath MojString deviceName
    err = getByDeviceUuid(shardInfo.deviceId, databaseShardInfo, found);
    MojErrCheck(err);

    if (found) {    // shard already registered in database
        copyRequiredFields(shardInfo, databaseShardInfo);
        err = m_mediaLinkManager.processShardInfo(databaseShardInfo);
        MojErrCheck(err);

        err = removeTransientShard(databaseShardInfo);
        MojErrCheck(err);

        err = update(databaseShardInfo);
        MojErrCheck(err);
    } else {        // not found in database
        err = allocateId(shardInfo.deviceId, databaseShardInfo.id);
        MojErrCheck(err);
        LOG_DEBUG("[db_shardEngine] shardEngine for device %s generated shard id: %d", databaseShardInfo.deviceId.data(), databaseShardInfo.id);

        copyRequiredFields(shardInfo, databaseShardInfo);
        databaseShardInfo.deviceId = shardInfo.deviceId;
        databaseShardInfo.transient = false;

        err = m_mediaLinkManager.processShardInfo(databaseShardInfo);
        MojErrCheck(err);

        err = put(databaseShardInfo);
        MojErrCheck(err);
    }
    return MojErrNone;
}

MojErr MojDbShardEngine::copyRequiredFields(const MojDbShardInfo& from, MojDbShardInfo& to)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    to.deviceUri = from.deviceUri;
    to.deviceName = from.deviceName;
    to.active = from.active;

    return MojErrNone;
}

MojErr MojDbShardEngine::removeTransientShard(const MojDbShardInfo& shardInfo)
{
    MojErr err;
    if (!shardInfo.active && shardInfo.transient) {
        bool found;
        err = isIdExist(shardInfo.id, found);
        MojErrCheck(err);

        if(found) {
            err = removeShardObjects(shardInfo.id_base64);
            MojErrCheck(err);
            err = removeShardInfo(shardInfo.id);
            MojErrCheck(err);
        }
    }

    return MojErrNone;
}
