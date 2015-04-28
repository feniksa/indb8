/* @@@LICENSE
*
* Copyright (c) 2013-2014 LG Electronics, Inc.
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

#ifndef MOJDBLEVELINDEX_H
#define MOJDBLEVELINDEX_H

#include "db/MojDbStorageEngine.h"
#include "MojDbSandwichDatabase.h"
#include "db/MojDbStorageIndex.h"

class MojDbSandwichIndex : public MojDbStorageIndex
{
public:
    MojDbSandwichIndex();
    virtual ~MojDbSandwichIndex();

    MojErr open(const MojObject& id, MojDbSandwichDatabase* db, MojDbStorageTxn* txn);
    virtual MojErr close();
    virtual MojErr drop(MojDbStorageTxn* txn);
    virtual MojErr stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut);
    virtual MojErr insert(const MojDbKey& key, MojDbStorageTxn* txn);
    virtual MojErr del(const MojDbKey& key, MojDbStorageTxn* txn);
    virtual MojErr find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut);
    virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut);

private:
    bool isOpen() const { return m_primaryDb.get() != NULL; }

    MojObject m_id;
    MojRefCountedPtr<MojDbSandwichDatabase> m_primaryDb;
    MojRefCountedPtr<MojDbSandwichDatabase> m_db;
};

#endif
