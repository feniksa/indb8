/* @@@LICENSE
*
* Copyright (c) 2009-2014 LG Electronics, Inc.
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

#pragma once

#include <leveldb/db.h>
#include "core/MojThread.h"
#include "db/MojDbDefs.h"
#include "db/MojDbStorageEngine.h"

#include "leveldb/sandwich_db.hpp"
#include "leveldb/bottom_db.hpp"
#include "leveldb/memory_db.hpp"

class MojDbSandwichDatabase;
class MojDbSandwichEnv;
class MojDbSandwichSeq;

class MojDbSandwichEngine final : public MojDbStorageEngine
{
public:
    typedef leveldb::SandwichDB<leveldb::BottomDB> BackendDb;
    MojDbSandwichEngine();
	MojDbSandwichEngine(MojRefCountedPtr<MojDbSandwichEnv>& env);
    ~MojDbSandwichEngine();

    virtual MojErr configure(const MojObject& config);
    virtual MojErr drop(const MojChar* path, MojDbStorageTxn* txn);
    virtual MojErr open(const MojChar* path);
    virtual MojErr open(const MojChar* path, MojDbEnv* env);
    virtual MojErr close();
    virtual MojErr compact();
    virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut);
    virtual MojErr openDatabase(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageDatabase>& dbOut) ;
    virtual MojErr openSequence(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageSeq>& seqOut) ;

    const MojString& path() const { return m_path; }
    MojDbSandwichEnv* env() { return m_env.get(); }
    MojErr addSeq(MojDbSandwichSeq* seq);
    MojErr removeSeq(MojDbSandwichSeq* seq);

    MojDbSandwichDatabase* indexDb() { return m_indexDb.get(); }
    BackendDb& impl() {return m_db;}

    static const leveldb::WriteOptions& getWriteOptions() { return WriteOptions; }
    static const leveldb::ReadOptions& getReadOptions() { return ReadOptions; }
    static const leveldb::Options& getOpenOptions() { return OpenOptions; }
private:
    typedef MojVector<MojRefCountedPtr<MojDbSandwichDatabase> > DatabaseVec;
    typedef MojVector<MojRefCountedPtr<MojDbSandwichSeq> > SequenceVec;

    BackendDb m_db;
    MojRefCountedPtr<MojDbSandwichEnv> m_env;
    MojThreadMutex m_dbMutex;
    MojRefCountedPtr<MojDbSandwichDatabase> m_indexDb;
    MojRefCountedPtr<MojDbSandwichDatabase> m_seqDb;
    MojString m_path;
    SequenceVec m_seqs;
    bool m_isOpen;

    static leveldb::ReadOptions ReadOptions;
    static leveldb::WriteOptions WriteOptions;
    static leveldb::Options OpenOptions;
};
