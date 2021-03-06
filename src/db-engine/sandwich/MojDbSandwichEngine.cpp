/* @@@LICENSE
*
*  Copyright (c) 2009-2015 LG Electronics, Inc.
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

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <leveldb/iterator.h>

#include "db-engine/sandwich/MojDbSandwichEngine.h"
#include "db-engine/sandwich/MojDbSandwichFactory.h"
#include "db-engine/sandwich/MojDbSandwichDatabase.h"
#include "db-engine/sandwich/MojDbSandwichQuery.h"
#include "db-engine/sandwich/MojDbSandwichSeq.h"
#include "db-engine/sandwich/MojDbSandwichTxn.h"
#include "db-engine/sandwich/MojDbSandwichEnv.h"
#include "db-engine/sandwich/defs.h"

#include "db/MojDbObjectHeader.h"
#include "db/MojDbQueryPlan.h"
#include "db/MojDb.h"
#include "core/MojObjectBuilder.h"
#include "core/MojObjectSerialization.h"
#include "core/MojString.h"
#include "core/MojTokenSet.h"

#include <sys/statvfs.h>


//const MojChar* const MojDbSandwichEnv::LockFileName = _T("_lock");
//db.ldb
static const MojChar* const MojEnvIndexDbName = _T("indexes.ldb");
static const MojChar* const MojEnvSeqDbName = _T("seq.ldb");

leveldb::ReadOptions MojDbSandwichEngine::ReadOptions;
leveldb::WriteOptions MojDbSandwichEngine::WriteOptions;
leveldb::Options MojDbSandwichEngine::OpenOptions;


////////////////////MojDbSandwichEngine////////////////////////////////////////////

MojDbSandwichEngine::MojDbSandwichEngine()
: m_isOpen(false)
{
}

MojDbSandwichEngine::MojDbSandwichEngine(MojRefCountedPtr<MojDbSandwichEnv>& env)
:m_env(env),
 m_isOpen(false)
{
}

MojDbSandwichEngine::~MojDbSandwichEngine()
{
    MojErr err =  close();
    MojErrCatchAll(err);
}

MojErr MojDbSandwichEngine::configure(const MojObject& config)
{
	if (!config.get("sync", WriteOptions.sync)) {
		WriteOptions.sync = true;
	}

    if (!config.get("fill_cache", ReadOptions.fill_cache)) {
        ReadOptions.fill_cache = true;
    }

    if (!config.get("verify_checksums", ReadOptions.verify_checksums)) {
        ReadOptions.verify_checksums = false;
    }

    if (!config.get("paranoid_checks", OpenOptions.paranoid_checks)) {
        OpenOptions.paranoid_checks = true;
    }

    OpenOptions.create_if_missing = true;

    return MojErrNone;
}

// although we might have only 1 real Level DB - just to be safe and consistent
// provide interface to add/drop multiple DB
MojErr MojDbSandwichEngine::drop(const MojChar* path, MojDbStorageTxn* txn)
{
    MojAssert(m_isOpen);

    MojThreadGuard guard(m_dbMutex);

    // close sequences
    for (SequenceVec::ConstIterator i = m_seqs.begin(); i != m_seqs.end(); ++i) {
        MojErr err = (*i)->close();
        MojErrCheck(err);
    }
    m_seqs.clear();

    // TODO: drop transaction
	// Must drop database by path! Throw as not implemented error

	MojErrThrowMsg(MojErrNotImplemented, _T("Not fully implemented")) ;
}
MojErr MojDbSandwichEngine::open(const MojChar* path)
{
    MojAssert(path);
    MojAssert(m_env.get());
	MojAssert(!m_isOpen);

	MojErr err;

	err = m_path.assign(path);
	MojErrCheck(err);

	// TODO: We should get this options from ENV
	m_db->options = MojDbSandwichEngine::getOpenOptions();
	m_db->writeOptions = MojDbSandwichEngine::getWriteOptions();
	m_db->readOptions = MojDbSandwichEngine::getReadOptions();

	leveldb::Status status = m_db->Open(path);
	MojLdbErrCheck(status, _T("db_create/db_open"));

	// open seqence db
	m_seqDb.reset(new MojDbSandwichDatabase(m_db.use(MojEnvSeqDbName)));
	MojAllocCheck(m_seqDb.get());

	err = m_seqDb->open(MojEnvSeqDbName, this);
	MojErrCheck(err);

	// open index db
	m_indexDb.reset(new MojDbSandwichDatabase(m_db.use(MojEnvIndexDbName)));
	MojAllocCheck(m_indexDb.get());

	err = m_indexDb->open(MojEnvIndexDbName, this);
	MojErrCheck(err);

	m_isOpen = true;

    return MojErrNone;
}

MojErr MojDbSandwichEngine::open(const MojChar* path, MojDbEnv* env)
{
	MojAssert(env);

	MojErr err;

    MojDbSandwichEnv* bEnv = static_cast<MojDbSandwichEnv *> (env);
    MojAssert(bEnv);

    m_env.reset(bEnv);

	err = open(path);
	MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbSandwichEngine::close()
{
    MojErr err = MojErrNone;
    MojErr errClose = MojErrNone;

    // close seqs before closing their databases
    m_seqs.clear();

    // close dbs
    if (m_seqDb.get()) {
        errClose = m_seqDb->close();
        MojErrAccumulate(err, errClose);
        m_seqDb.reset();
    }
    if (m_indexDb.get()) {
        errClose = m_indexDb->close();
        MojErrAccumulate(err, errClose);
        m_indexDb.reset();
    }
    m_env.reset();
    m_isOpen = false;

    return err;
}

MojErr MojDbSandwichEngine::beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
{
    MojAssert(!txnOut.get());

    MojRefCountedPtr<MojDbSandwichEnvTxn> txn(new MojDbSandwichEnvTxn(m_db));
    MojAllocCheck(txn.get());
    txnOut = txn;

    return MojErrNone;
}

MojErr MojDbSandwichEngine::openDatabase(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageDatabase>& dbOut)
{
    MojAssert(name && !dbOut.get());

    BackendDb::Cookie cookie;

    leveldb::Status status = m_db.cook(name, cookie);
    MojLdbErrCheck(status, "openDatabase");

    MojRefCountedPtr<MojDbSandwichDatabase> db(new MojDbSandwichDatabase(m_db.use(cookie)));
    MojAllocCheck(db.get());

    MojErr err = db->open(name, this);
    MojErrCheck(err);

    dbOut = db;

    return MojErrNone;
}

MojErr MojDbSandwichEngine::openSequence(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageSeq>& seqOut)
{
    MojAssert(name && !seqOut.get());
    MojAssert(m_seqDb.get());

    MojRefCountedPtr<MojDbSandwichSeq> seq(new MojDbSandwichSeq());
    MojAllocCheck(seq.get());


    MojErr err = seq->open(name, m_seqDb.get());
    MojErrCheck(err);
    seqOut = seq;
    m_seqs.push(seq);

    return MojErrNone;
}


// placeholder
MojErr MojDbSandwichEngine::compact()
{
    MojThreadGuard guard(m_dbMutex);

    (*m_db)->CompactRange(nullptr, nullptr);

    return MojErrNone;
}

MojErr MojDbSandwichEngine::addSeq(MojDbSandwichSeq* seq)
{
    MojAssert(seq);

    MojThreadGuard guard(m_dbMutex);

    return m_seqs.push(seq);
}

MojErr MojDbSandwichEngine::removeSeq(MojDbSandwichSeq* seq)
{
    MojAssert(seq);
    MojThreadGuard guard(m_dbMutex);

    MojSize idx;
    MojSize size = m_seqs.size();
    for (idx = 0; idx < size; ++idx) {
        if (m_seqs.at(idx).get() == seq) {
            MojErr err = m_seqs.erase(idx);
            MojErrCheck(err);
            break;
        }
    }
    return MojErrNone;
}
