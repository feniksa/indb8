/* @@@LICENSE
*
*  Copyright (c) 2009-2014 LG Electronics, Inc.
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

#include "db-engine/sandwich/MojDbSandwichIndex.h"
#include "db-engine/sandwich/MojDbSandwichEngine.h"
#include "db-engine/sandwich/MojDbSandwichQuery.h"
#include "db-engine/sandwich/MojDbSandwichTxn.h"
#include "db-engine/sandwich/defs.h"

////////////////////MojDbIndex////////////////////////////////////////////

MojDbSandwichIndex::MojDbSandwichIndex()
: m_primaryDb(NULL)
{
}

MojDbSandwichIndex::~MojDbSandwichIndex()
{
    MojErr err =  close();
    MojErrCatchAll(err);
}

MojErr MojDbSandwichIndex::open(const MojObject& id, MojDbSandwichDatabase* db, MojDbStorageTxn* txn)
{
    MojAssert(db && db->engine());

    m_id = id;

    // this is deprecated
    // m_db and m_primaryDb should point to the same object
    // leave both of them here more for debugging purposes

    m_db.reset(db->engine()->indexDb());
    m_primaryDb.reset(db);

    return MojErrNone;
}

MojErr MojDbSandwichIndex::close()
{
    m_db.reset();
    m_primaryDb.reset();

    return MojErrNone;
}

MojErr MojDbSandwichIndex::drop(MojDbStorageTxn* abstractTxn)
{
    MojAssert( dynamic_cast<MojDbSandwichEnvTxn *>(abstractTxn) );

    auto txn = static_cast<MojDbSandwichEnvTxn *>(abstractTxn);

    MojDbKey prefixKey;
    MojErr err = prefixKey.assign(m_id);
    MojErrCheck(err);

    err = m_db->delPrefix(*txn, { (const char *)prefixKey.data(), prefixKey.size() });
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbSandwichIndex::stats(MojDbStorageTxn* abstractTxn, MojSize& countOut, MojSize& sizeOut)
{
    MojAssert( dynamic_cast<MojDbSandwichEnvTxn *>(abstractTxn) );

    auto txn = static_cast<MojDbSandwichEnvTxn *>(abstractTxn);

    MojDbKey prefixKey;
    MojErr err = prefixKey.assign(m_id);
    MojErrCheck(err);

    err = m_db->stats(txn, countOut, sizeOut, { (const char*)prefixKey.data(), prefixKey.size() });
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbSandwichIndex::insert(const MojDbKey& key, MojDbStorageTxn* txn)
{
    MojAssert(txn);

    MojDbSandwichItem keyItem;
    keyItem.fromBytesNoCopy(key.data(), key.size());
    MojDbSandwichItem valItem;  // empty item? not clear why do we need to insert it
    MojErr err = m_db->put(keyItem, valItem, txn, true);
#ifdef MOJ_DEBUG
    char s[1024];
    size_t size1 = keyItem.size();
    MojErr err2 = MojByteArrayToHex(keyItem.data(), size1, s);
    MojErrCheck(err2);
    if (size1 > 16) // if the object-id is in key
        strncat(s, (char *)(keyItem.data()) + (size1 - 17), 16);
#endif
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbSandwichIndex::del(const MojDbKey& key, MojDbStorageTxn* txn)
{
    MojAssert(txn);
    MojAssert(isOpen());

    MojDbSandwichItem keyItem;
    keyItem.fromBytesNoCopy(key.data(), key.size());

    bool found = false;
    MojErr err = m_db->del(keyItem, found, txn);

#ifdef MOJ_DEBUG
    char s[1024];
    size_t size1 = keyItem.size();
    MojErr err2 = MojByteArrayToHex(keyItem.data(), size1, s);
    MojErrCheck(err2);
    if (size1 > 16) // if the object-id is in key
        strncat(s, (char *)(keyItem.data()) + (size1 - 17), 16);
	MojAssert(found);
#endif

    MojErrCheck(err);
    if (!found) {
        //MojErrThrow(MojErrDbInconsistentIndex);   // fix this to work around to deal with out of sync indexes
        MojErrThrow(MojErrInternalIndexOnDel);
    }
    return MojErrNone;
}

MojErr MojDbSandwichIndex::find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut)
{
    MojAssert(isOpen());
    MojAssert(plan.get() && txn);

    MojRefCountedPtr<MojDbSandwichQuery> storageQuery(new MojDbSandwichQuery());
    MojAllocCheck(storageQuery.get());
    MojErr err = storageQuery->open(m_db.get(), m_primaryDb.get(), plan, txn);
    MojErrCheck(err);
    queryOut = storageQuery;

    return MojErrNone;
}

MojErr MojDbSandwichIndex::beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
{
    return m_primaryDb->beginTxn(txnOut);
}
