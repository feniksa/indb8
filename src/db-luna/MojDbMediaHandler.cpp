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

#include "db-luna/MojDbMediaHandler.h"
#include "db/MojDbServiceDefs.h"
#include "db/MojDb.h"

MojDbMediaHandler::MojDbMediaHandler(MojService& service, MojDb& db)
: m_deviceListSlot(this, &MojDbMediaHandler::handleDeviceListResponse),
  m_service(service),
  m_db(db)
{
}

MojErr MojDbMediaHandler::subscribe()
{
    MojObject payload;
    MojErr err = payload.put(_T("subscribe"), true);
    MojErrCheck(err);
    err = m_service.createRequest(m_subscription);
    MojErrCheck(err);
    err = m_subscription->send(m_deviceListSlot, MojDbServiceDefs::PDMServiceName, _T("listDevices"), payload, MojServiceRequest::Unlimited);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbMediaHandler::handleDeviceListResponse(MojObject& payload, MojErr errCode)
{
    MojErr err;
    if (errCode != MojErrNone) {
        MojString payloadStr;
        err = payload.stringValue(payloadStr);
        MojErrCheck(err);
    
        MojErrThrow(errCode);
    }
    MojObject deviceList;
    err = payload.getRequired("devices", deviceList);
    MojErrCheck(err);

    std::set<MojString> shardIds;           // list of not processed ids of already know
    copyShardCache(&shardIds);

    for (MojObject::ConstArrayIterator i = deviceList.arrayBegin(); i != deviceList.arrayEnd(); ++i) {
        MojString deviceType;
        MojObject subDevices;

        err = i->getRequired("deviceType", deviceType);
        MojErrCheck(err);

        if (deviceType != "usb") {
            // not usb media. PDM returns ALL list of media, like USB, Internal storage and other.
            // db8 intresting only in usb sticks
            continue;
        }

        err = i->getRequired("subDevices", subDevices);
        MojErrCheck(err);

        for (MojObject::ConstArrayIterator it = subDevices.arrayBegin(); it != subDevices.arrayEnd(); ++it) {
            MojDbShardInfo shardInfo;
            err = convert(*it, shardInfo);
            MojErrCheck(err);

            if (shardInfo.deviceId.empty()) {
                continue;
            }

            shardInfo.active = true;

            shardIds.erase(shardInfo.deviceId);  // mark it as processed

            if (!existInCache(shardInfo.deviceId)) {
                m_shardCache[shardInfo.deviceId] = shardInfo;
                err = m_db.shardEngine()->processShardInfo(shardInfo);
                MojErrCheck(err);
            }
       } // end subDevices loop
    }   // end main list of devices loop

    // notify shard engine about all inactive shards
    for (std::set<MojString>::const_iterator i = shardIds.begin(); i != shardIds.end(); ++i) {
        shard_cache_t::iterator shardCacheIterator = m_shardCache.find(*i);
        if (shardCacheIterator != m_shardCache.end()) {
            shardCacheIterator->second.active = false;
            err = m_db.shardEngine()->processShardInfo(shardCacheIterator->second);
            MojErrCheck(err);
            m_shardCache.erase(shardCacheIterator);
        }
    }

    /*bool finished = false;
    MojErr err = payload.getRequired(_T("finished"), finished);
    MojErrCheck(err);
    if (finished) {
        m_subscription.reset();
        MojRefCountedPtr<MojServiceRequest> req;
        err = m_service.createRequest(req);
        MojErrCheck(err);
        err = req->send(m_alertSlot, _T("com.palm.systemmanager"), _T("publishToSystemUI"), m_payload);
        MojErrCheck(err);
    }*/
    return MojErrNone;
}

MojErr MojDbMediaHandler::convert(const MojObject& object, MojDbShardInfo& shardInfo)
{
    MojErr err;
    err = object.getRequired("deviceId", shardInfo.deviceId);
    MojErrCheck(err);

    err = object.getRequired("deviceName", shardInfo.deviceName);
    MojErrCheck(err);

    err = object.getRequired("deviceUri", shardInfo.deviceUri);
    MojErrCheck(err);

    return MojErrNone;
}

bool MojDbMediaHandler::existInCache(const MojString& id)
{
    shard_cache_t::iterator i = m_shardCache.find(id);
    return (i != m_shardCache.end());
}

void MojDbMediaHandler::copyShardCache(std::set<MojString>* shardIdSet)
{
    for (shard_cache_t::const_iterator i = m_shardCache.begin(); i != m_shardCache.end(); ++i) {
        shardIdSet->insert(i->first);
    }
}
