/* @@@LICENSE
*
* Copyright (c) 2013-2015 LG Electronics, Inc.
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

#ifndef MOJDBLEVELDATABASE_H
#define MOJDBLEVELDATABASE_H

#include <leveldb/db.h>
#include "db/MojDbDefs.h"
#include "db/MojDbStorageDatabase.h"
#include "MojDbSandwichEngine.h"

class MojDbSandwichItem;
class MojDbSandwichEnvTxn;

class MojDbSandwichDatabase final : public MojDbStorageDatabase
{
public:
    MojDbSandwichDatabase(const MojDbSandwichEngine::BackendDb::Part& part);
    ~MojDbSandwichDatabase();

    MojErr open(const MojChar* dbName, MojDbSandwichEngine* env);
    MojErr close() override;
    MojErr drop(MojDbStorageTxn* txn) override;
    MojErr stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut) override;
    MojErr insert(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn) override;
    MojErr update(const MojObject& id, MojBuffer& val, MojDbStorageItem* oldVal, MojDbStorageTxn* txn) override;
    MojErr del(const MojObject& id, MojDbStorageTxn* txn, bool& foundOut) override;
    MojErr get(const MojObject& id, MojDbStorageTxn* txn, bool forUpdate, MojRefCountedPtr<MojDbStorageItem>& itemOut) override;
    MojErr find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut) override;
    MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut) override;
    MojErr openIndex(const MojObject& id, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageIndex>& indexOut) override;

//hack:
    MojErr mutexStats(int* total_mutexes, int* mutexes_free, int* mutexes_used, int* mutexes_used_highwater, int* mutex_regionsize) override;

    MojErr put(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn, bool updateIdQuota);
    MojErr put(MojDbSandwichItem& key, MojDbSandwichItem& val, MojDbStorageTxn* txn, bool updateIdQuota);
    MojErr del(MojDbSandwichItem& key, bool& foundOut, MojDbStorageTxn* txn);
    MojErr get(MojDbSandwichItem& key, MojDbStorageTxn* txn, bool forUpdate, MojDbSandwichItem& valOut, bool& foundOut);
    void   compact();

    MojErr delPrefix(MojDbSandwichEnvTxn &txn, leveldb::Slice prefix);
    MojErr stats(MojDbSandwichEnvTxn* txn, MojSize &countOut, MojSize &sizeOut, leveldb::Slice prefix);

    MojDbSandwichEngine::BackendDb::Part& impl() { return m_db; }
    MojDbSandwichEngine* engine() { return m_engine; }

    leveldb::DB* getDb();

private:
    friend class MojDbSandwichEngine;
    friend class MojDbSandwichIndex;

    //MojErr verify();
    MojErr closeImpl();
    void postUpdate(MojDbStorageTxn* txn, MojSize updateSize);

    MojDbSandwichEngine::BackendDb::Part m_db;
    MojDbSandwichEngine* m_engine;
    MojString m_name;
};

#endif
