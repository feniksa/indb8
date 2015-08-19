/* @@@LICENSE
*
* Copyright (c) 2009-2013 LG Electronics, Inc.
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


#include "db-engine/berkeley/MojDbBerkeleyEngine.h"
#include "db-engine/berkeley/MojDbBerkeleyFactory.h"
#include "db-engine/berkeley/MojDbBerkeleyQuery.h"
#include "db-engine/berkeley/MojDbBerkeleySeq.h"
#include "db-engine/berkeley/MojDbBerkeleyDatabase.h"
#include "db-engine/berkeley/MojDbBerkeleyEnv.h"
#include "db-engine/berkeley/MojDbBerkeleyTxn.h"

#include "db/MojDbObjectHeader.h"
#include "db/MojDbQueryPlan.h"
#include "db/MojDb.h"
#include "core/MojObjectBuilder.h"
#include "core/MojObjectSerialization.h"
#include "core/MojString.h"
#include "core/MojTokenSet.h"

#include <sys/statvfs.h>

MojLogger MojDbBerkeleyEngine::s_log(_T("db.bdb"));

MojDbBerkeleyEngine::MojDbBerkeleyEngine()
: m_isOpen(false)
{
	MojLogTrace(s_log);
}

MojDbBerkeleyEngine::MojDbBerkeleyEngine(MojRefCountedPtr<MojDbBerkeleyEnv>& env)
: m_env(env),
  m_isOpen(false)
{
	MojLogTrace(s_log);
}

MojDbBerkeleyEngine::~MojDbBerkeleyEngine()
{
	MojLogTrace(s_log);

	MojErr err =  close();
	MojErrCatchAll(err);
}

MojErr MojDbBerkeleyEngine::configure(const MojObject& conf)
{
	MojLogTrace(s_log);

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::drop(const MojChar* path, MojDbStorageTxn* txn)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);
	MojAssert(m_isOpen);

	MojThreadGuard guard(m_dbMutex);

	// close sequences
	for (SequenceVec::ConstIterator i = m_seqs.begin(); i != m_seqs.end(); ++i) {
		MojErr err = (*i)->closeImpl();
		MojErrCheck(err);
	}
	m_seqs.clear();
	// drop databases
	for (DatabaseVec::ConstIterator i = m_dbs.begin(); i != m_dbs.end(); ++i) {
		MojErr err = (*i)->closeImpl();
		MojErrCheck(err);
		err = (*i)->drop(txn);
		MojErrCheck(err);
	}
	m_dbs.clear();

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::open(const MojChar* path)
{
	MojAssert(path);
	MojAssert(!m_env.get() && !m_isOpen);
	MojLogTrace(s_log);

	MojRefCountedPtr<MojDbBerkeleyEnv> env(new MojDbBerkeleyEnv);
	MojAllocCheck(env.get());
	MojErr err = env->open(path);
	MojErrCheck(err);
	err = open(NULL, env.get());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::open(const MojChar* path, MojDbEnv* env)
{
    MojDbBerkeleyEnv* bEnv = static_cast<MojDbBerkeleyEnv *> (env);
    MojAssert(bEnv);
	MojAssert(!m_env.get() && !m_isOpen);
	MojLogTrace(s_log);


	m_env.reset(bEnv);
	if (path) {
		MojErr err = m_path.assign(path);
		MojErrCheck(err);
		// create dir
		err = MojCreateDirIfNotPresent(path);
		MojErrCheck(err);
	}
	// open seqence db
	bool created = false;
	m_seqDb.reset(new MojDbBerkeleyDatabase);
	MojAllocCheck(m_seqDb.get());
	MojErr err = m_seqDb->open(defs::MojEnvSeqDbName, this, created, NULL);
	MojErrCheck(err);
	// open index db
	m_indexDb.reset(new MojDbBerkeleyDatabase);
	MojAllocCheck(m_indexDb.get());
	err = m_indexDb->open(defs::MojEnvIndexDbName, this, created, NULL);
	MojErrCheck(err);
	m_isOpen = true;

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::close()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);
	MojErr err = MojErrNone;
	MojErr errClose = MojErrNone;

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

MojErr MojDbBerkeleyEngine::compact()
{
	const char * DatabaseRoot = "/var/db"; // FIXME: Should not be hard-coded, but so is the disk space monitor!

	struct statvfs statAtBeginning, statAfterCompact, statAtEnd;

	MojLogTrace(MojDbBerkeleyEngine::s_log);

	struct timeval totalStartTime = {0,0}, totalStopTime = {0,0};

	gettimeofday(&totalStartTime, NULL);

	memset(&statAtBeginning, '\0', sizeof(statAtBeginning));
	::statvfs(DatabaseRoot, &statAtBeginning);

	const int blockSize = (int)statAtBeginning.f_bsize;

	// checkpoint before compact
	MojErr err = m_env->checkpoint(0);
	MojErrCheck(err);

	memset(&statAfterCompact, '\0', sizeof(statAfterCompact));
	::statvfs(DatabaseRoot, &statAfterCompact);

	int pre_compact_reclaimed_blocks = (int)(statAfterCompact.f_bfree - statAtBeginning.f_bfree);

    MojLogDebug(s_log, _T("Starting compact: Checkpoint freed %d bytes. Volume %s has %lu bytes free out of %lu bytes (%.1f full)\n"),
		pre_compact_reclaimed_blocks * blockSize,
		DatabaseRoot, statAfterCompact.f_bfree * blockSize,
		 statAfterCompact.f_blocks * blockSize,
		 (float)(statAfterCompact.f_blocks - statAfterCompact.f_bfree) * 100.0 / (float)statAfterCompact.f_blocks);


	// Retrieve setting for record count used to break up compact operations
	const int stepSize = m_env->compactStepSize();

	memset(&statAtBeginning, '\0', sizeof(statAtBeginning));
	::statvfs(DatabaseRoot, &statAtBeginning);

	int total_pages_examined = 0, total_pages_freed = 0, total_pages_truncated = 0;
	int max_pages_examined = 0, max_pages_freed = 0, max_pages_truncated = 0;
	int total_log_generation_blocks = 0, total_reclaimed_blocks = 0;
	int max_log_generation_blocks = 0, max_reclaimed_blocks = 0;

	int total_compact_time = 0, total_step_time = 0;
	int max_compact_time = 0, max_step_time = 0;

	int total_key_total = 0, total_value_total = 0;
	int max_key_total = 0, max_value_total = 0;

	MojThreadGuard guard(m_dbMutex);
	// call compact on each database
	for (DatabaseVec::ConstIterator i = m_dbs.begin(); i != m_dbs.end(); ++i) {
		DB* db = (*i)->impl();
		DB_COMPACT c_data;
		MojZero(&c_data, sizeof(c_data));

		DBC * dbc = NULL;
		int dbErr;
		DBT key1, key2;
		DBT value;

		memset(&key1, '\0', sizeof(key1));
		memset(&key2, '\0', sizeof(key2));
		memset(&value, '\0', sizeof(value));
		key1.flags = DB_DBT_REALLOC;
		key2.flags = DB_DBT_REALLOC;
		value.flags = DB_DBT_REALLOC;

		int key1_count = 0, key2_count = 0;

		dbErr = 0;

		// Continue compacting the database by chunks until we run into an error. If a stepSize
		// isn't configured, don't chunk it at all.
		while ((stepSize >= 1) && (dbErr == 0)) {

			// Construct key to step forward by a set number of records, to select the compact window.
			// We close the cursor after we've found the next key, so it won't keep a lock open that
			// could disrupt the compaction. Without locking, we might miss an insertion or deletion
			// happening between compactions, but that

			int key_total = 0, value_total = 0; // Tracked only for debugging purposes.

			dbErr = db->cursor(db, NULL, &dbc, 0);

			if (dbErr == 0) {

				if (key1.data == NULL) {
					// Move the cursor to the beginning of the database
					dbErr = dbc->get(dbc, &key1, &value, DB_FIRST);

					key_total += key1.size;
					value_total += value.size;

					// discard key1, we don't want the key for the beginning
					if (key1.data)
						free(key1.data);

					key1.data = NULL;
					key1.size = 0;

				} else {
					// move the cursor to the location of the prior key.
					// If that exact key is missing, this should choose the
					// next one.
					dbErr = dbc->get(dbc, &key1, &value, DB_SET_RANGE);
				}

				int elapsedStepTimeMS = 0;

				if (dbErr == DB_NOTFOUND) {
					// If we didn't find a first key, the DB is presumably empty,
					// and we shouldn't search for the end key.

					dbErr = 0;

					if (key1.data)
						free(key1.data);
					key1.data = NULL;
					key1.size = 0;

					if (key2.data)
						free(key2.data);
					key2.data = NULL;
					key2.size = 0;

				} else if (dbErr == 0) {

					int count;
					// Move the cursor forward by the chosen stepSize.
					// May exit early with error DB_NOTFOUND, indicating end of database.

					struct timeval startTime = {0,0}, stopTime = {0,0};

					gettimeofday(&startTime, NULL);

					for (count = 0; (dbErr == 0) && (count < stepSize); count++) {
						dbErr = dbc->get(dbc, &key2, &value, DB_NEXT);

						key_total += key2.size;
						value_total += value.size;
					}

					key2_count = key1_count + count;

					if (dbErr == DB_NOTFOUND) {
						dbErr = 0;

						if (key2.data)
							free(key2.data);
						key2.data = NULL;
						key2.size = 0;
					}

					gettimeofday(&stopTime, NULL);

					elapsedStepTimeMS = (int)(stopTime.tv_sec - startTime.tv_sec) * 1000 +
							  (int)(stopTime.tv_usec - startTime.tv_usec) / 1000;
				}

				dbc->close(dbc);

				if (dbErr != 0)
					break;

				// Compact from key1 to key2. (The documentation says it starts at 'the
				// smallest key greater than or equal to the specified key', and ends at
				// 'the page with the smallest key greater than the specified key'. I don't
				// know exactly what that means regarding inclusivity, so this procedure may
				// not be fully compacting the pages which contain the keys.)


				MojLogDebug(s_log, _T("Compacting %s (partial from ~record %d to %d). Stepped over %d/%d bytes of keys/values in %dms.\n"), (*i)->m_name.data(),
					key1_count, key2_count,
					key_total, value_total,
					elapsedStepTimeMS);

			        struct statvfs statBeforeCompact, statAfterCompact, statAfterCheckpoint;

				memset(&statBeforeCompact, '\0', sizeof(statBeforeCompact));
				::statvfs(DatabaseRoot, &statBeforeCompact);

				struct timeval startTime = {0,0}, stopTime = {0,0};

				gettimeofday(&startTime, NULL);

				MojZero(&c_data, sizeof(c_data));
				dbErr = db->compact(db, NULL, key1.data ? &key1 : NULL, key2.data ? &key2 : NULL, &c_data, DB_FREE_SPACE, NULL);

				gettimeofday(&stopTime, NULL);

				int elapsedCompactTimeMS = (int)(stopTime.tv_sec - startTime.tv_sec) * 1000 +
						           (int)(stopTime.tv_usec - startTime.tv_usec) / 1000;

		                MojLogDebug(s_log, _T("Compact stats of %s (partial from ~record %d to %d): time %dms, compact_deadlock=%d, compact_pages_examine=%d, compact_pages_free=%d, compact_levels=%d, compact_pages_truncated=%d\n"),
        		        	(*i)->m_name.data(),
        		        	key1_count, key2_count,
        		        	elapsedCompactTimeMS,
                			c_data.compact_deadlock, c_data.compact_pages_examine,
               			 	c_data.compact_pages_free, c_data.compact_levels, c_data.compact_pages_truncated);

				total_compact_time += elapsedCompactTimeMS;
				if (elapsedCompactTimeMS > max_compact_time)
					max_compact_time = elapsedCompactTimeMS;
				total_step_time += elapsedStepTimeMS;
				if (elapsedStepTimeMS > max_step_time)
					max_step_time = elapsedStepTimeMS;

				total_key_total += key_total;
				if (key_total > max_key_total)
					max_key_total = key_total;
				total_value_total += value_total;
				if (value_total > max_value_total)
					max_value_total = value_total;

				total_pages_examined += c_data.compact_pages_examine;
				if ((int)c_data.compact_pages_examine > max_pages_examined)
					max_pages_examined = c_data.compact_pages_examine;
				total_pages_freed += c_data.compact_pages_free;
				if ((int)c_data.compact_pages_free > max_pages_freed)
					max_pages_freed = c_data.compact_pages_free;
				total_pages_truncated += c_data.compact_pages_truncated;
				if ((int)c_data.compact_pages_truncated > max_pages_truncated)
					max_pages_truncated = c_data.compact_pages_truncated;

				memset(&statAfterCompact, '\0', sizeof(statAfterCompact));
				::statvfs(DatabaseRoot, &statAfterCompact);

				int log_generation_blocks = (int)(statBeforeCompact.f_bfree - statAfterCompact.f_bfree);

				total_log_generation_blocks += log_generation_blocks;
				if (log_generation_blocks > max_log_generation_blocks)
					max_log_generation_blocks = log_generation_blocks;

				err = m_env->checkpoint(0);
				MojErrCheck(err);

				memset(&statAfterCompact, '\0', sizeof(statAfterCheckpoint));
				::statvfs(DatabaseRoot, &statAfterCheckpoint);

                int reclaimed_blocks = (int)(statAfterCheckpoint.f_bfree - statBeforeCompact.f_bfree);

				total_reclaimed_blocks += reclaimed_blocks;
				if (reclaimed_blocks > max_reclaimed_blocks)
					max_reclaimed_blocks = reclaimed_blocks;

				MojLogDebug(s_log, _T("Compact of %s (partial from ~record %d to %d) generated %d bytes of log data, ultimately reclaiming %d bytes after checkpoint.\n"),
					(*i)->m_name.data(),
					key1_count, key2_count,
					log_generation_blocks * blockSize,
					reclaimed_blocks * blockSize);

				// copy key2 over key1
				if (key1.data)
					free(key1.data);
				key1.data = key2.data;
				key1.size = key2.size;
				key2.data = NULL;
				key2.size = 0;
				key1_count = key2_count;

				// if key2 was empty, then we are done.
				if (key1.data == NULL)
					break;

			}


		}

		if (key1.data)
			free(key1.data);
		if (key2.data)
			free(key2.data);
		if (value.data)
			free(value.data);


		// If no step size was configured, fall back and do a complete compact. Do the same
		// if there was an error performing the chunked compaction. The complete compact risks
		// running out of disk space, but that's preferable to not compacting at all, which will
		// also likely eventually lead to running out of space.

		if (dbErr == DB_LOCK_DEADLOCK) {
			// But for deadlock, we should just give up, as this might
			// happen in normal use.
			MojBdbErrCheck(dbErr, _T("cursor and compact deadlocked"));
		}

		if ((stepSize <= 1) || (dbErr != 0)) {
            MojLogDebug(s_log, "Compacting %s\n", (*i)->m_name.data());

		        struct statvfs statBeforeCompact, statAfterCompact, statAfterCheckpoint;

			memset(&statBeforeCompact, '\0', sizeof(statBeforeCompact));
			::statvfs(DatabaseRoot, &statBeforeCompact);

			struct timeval startTime = {0,0}, stopTime = {0,0};

			gettimeofday(&startTime, NULL);

			MojZero(&c_data, sizeof(c_data));
		        dbErr = db->compact(db, NULL, NULL, NULL, &c_data, DB_FREE_SPACE, NULL);

			gettimeofday(&stopTime, NULL);

			int elapsedCompactTimeMS = (int)(stopTime.tv_sec - startTime.tv_sec) * 1000 +
					           (int)(stopTime.tv_usec - startTime.tv_usec) / 1000;

			total_compact_time += elapsedCompactTimeMS;
			if (elapsedCompactTimeMS > max_compact_time)
				max_compact_time = elapsedCompactTimeMS;

       	        	MojLogDebug(s_log, "Compact stats of %s: time %dms, compact_deadlock=%d, compact_pages_examine=%d, compact_pages_free=%d, compact_levels=%d, compact_pages_truncated=%d\n",
                		(*i)->m_name.data(),
                		elapsedCompactTimeMS,
                		c_data.compact_deadlock, c_data.compact_pages_examine,
                		c_data.compact_pages_free, c_data.compact_levels, c_data.compact_pages_truncated);

			total_pages_examined += c_data.compact_pages_examine;
			if ((int)c_data.compact_pages_examine > max_pages_examined)
				max_pages_examined = c_data.compact_pages_examine;
			total_pages_freed += c_data.compact_pages_free;
			if ((int)c_data.compact_pages_free > max_pages_freed)
				max_pages_freed = c_data.compact_pages_free;
			total_pages_truncated += c_data.compact_pages_truncated;
			if ((int)c_data.compact_pages_truncated > max_pages_truncated)
				max_pages_truncated = c_data.compact_pages_truncated;

			memset(&statAfterCompact, '\0', sizeof(statAfterCompact));
			::statvfs(DatabaseRoot, &statAfterCompact);

			int log_generation_blocks = (int)(statBeforeCompact.f_bfree - statAfterCompact.f_bfree);

			total_log_generation_blocks += log_generation_blocks;
			if (log_generation_blocks > max_log_generation_blocks)
				max_log_generation_blocks = log_generation_blocks;

			err = m_env->checkpoint(0);
			MojErrCheck(err);

			memset(&statAfterCompact, '\0', sizeof(statAfterCheckpoint));
			::statvfs(DatabaseRoot, &statAfterCheckpoint);

			int reclaimed_blocks = (int)(statAfterCheckpoint.f_bfree - statBeforeCompact.f_bfree);

			total_reclaimed_blocks += reclaimed_blocks;
			if (reclaimed_blocks > max_reclaimed_blocks)
				max_reclaimed_blocks = reclaimed_blocks;

            MojLogDebug(s_log, "Compact of %s generated %d bytes of log data, ultimately reclaiming %d bytes after checkpoint.\n",
				(*i)->m_name.data(),
				log_generation_blocks * blockSize,
				reclaimed_blocks * blockSize);
		}
		MojBdbErrCheck(dbErr, _T("db->compact"));

	}
	guard.unlock();


	gettimeofday(&totalStopTime, NULL);

	int elapsedTotalMS = (int)(totalStopTime.tv_sec - totalStartTime.tv_sec) * 1000 +
		             (int)(totalStopTime.tv_usec - totalStartTime.tv_usec) / 1000;

	memset(&statAtEnd, '\0', sizeof(statAtEnd));
	::statvfs(DatabaseRoot, &statAtEnd);

	int compact_freed_blocks = (int)(statAtEnd.f_bfree - statAtBeginning.f_bfree);

    MojLogDebug(s_log, _T("During compact: %d db pages examined (max burst %d), %d db pages freed (max burst %d), "
			     "%d db pages truncated (max burst %d), "
	                     "%d log bytes created by compacts (max burst %d), "
	                     "%d bytes reclaimed by checkpoints (max burst %d), "
	                     "%d bytes of keys stepped over (max burst %d), "
	                     "%d bytes of values stepped over (max burst %d), "
	                     "%dms spent in stepping (max burst %dms), "
	                     "%dms spent in compact (max burst %dms)\n"),
	                     total_pages_examined, max_pages_examined, total_pages_freed, max_pages_freed,
	                     total_pages_truncated, max_pages_truncated,
	                     total_log_generation_blocks * blockSize, max_log_generation_blocks * blockSize,
	                     total_reclaimed_blocks * blockSize, max_reclaimed_blocks * blockSize,
	                     total_key_total, max_key_total,
	                     total_value_total, max_value_total,
	                     total_step_time, max_step_time,
	                     total_compact_time, max_step_time
	                     );

    MojLogDebug(s_log, _T("Compact complete: took %dms, freed %d bytes (including pre-checkpoint of %d bytes). Volume %s has %lu bytes free out of %lu bytes (%.1f full)\n"),
		elapsedTotalMS,
		compact_freed_blocks * blockSize,
		pre_compact_reclaimed_blocks * blockSize,
		DatabaseRoot,
		statAfterCompact.f_bfree * blockSize,
		 statAfterCompact.f_blocks * blockSize,
		 (float)(statAfterCompact.f_blocks - statAfterCompact.f_bfree) * 100.0 / (float)statAfterCompact.f_blocks);

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
{
	MojAssert(!txnOut.get());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojRefCountedPtr<MojDbBerkeleyTxn> txn(new MojDbBerkeleyTxn());
	MojAllocCheck(txn.get());
	MojErr err = txn->begin(this);
	MojErrCheck(err);
	txnOut = txn;

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::openDatabase(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageDatabase>& dbOut)
{
	MojAssert(name && !dbOut.get());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojRefCountedPtr<MojDbBerkeleyDatabase> db(new MojDbBerkeleyDatabase());
	MojAllocCheck(db.get());
	bool created = false;
	MojErr err = db->open(name, this, created, txn);
	MojErrCheck(err);
	dbOut = db;

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::openSequence(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageSeq>& seqOut)
{
	MojAssert(name && !seqOut.get());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojRefCountedPtr<MojDbBerkeleySeq> seq(new MojDbBerkeleySeq());
	MojAllocCheck(seq.get());
	MojErr err = seq->open(name, m_seqDb.get());
	MojErrCheck(err);
	seqOut = seq;

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::addDatabase(MojDbBerkeleyDatabase* db)
{
	MojAssert(db);
	MojLogTrace(MojDbBerkeleyEngine::s_log);
	MojThreadGuard guard(m_dbMutex);

	return m_dbs.push(db);
}

MojErr MojDbBerkeleyEngine::removeDatabase(MojDbBerkeleyDatabase* db)
{
	MojAssert(db);
	MojLogTrace(MojDbBerkeleyEngine::s_log);
	MojThreadGuard guard(m_dbMutex);

	MojSize idx;
	MojSize size = m_dbs.size();
	for (idx = 0; idx < size; ++idx) {
		if (m_dbs.at(idx).get() == db) {
			MojErr err = m_dbs.erase(idx);
			MojErrCheck(err);
			break;
		}
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::addSeq(MojDbBerkeleySeq* seq)
{
	MojAssert(seq);
	MojLogTrace(MojDbBerkeleyEngine::s_log);
	MojThreadGuard guard(m_dbMutex);

	return m_seqs.push(seq);
}

MojErr MojDbBerkeleyEngine::removeSeq(MojDbBerkeleySeq* seq)
{
	MojAssert(seq);
	MojLogTrace(MojDbBerkeleyEngine::s_log);
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

MojDbBerkeleySeq::~MojDbBerkeleySeq()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err = close();
	MojErrCatchAll(err);
}

MojErr MojDbBerkeleySeq::open(const MojChar* name, MojDbBerkeleyDatabase* db)
{
	MojAssert(!m_seq);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	m_db = db;
	MojString strName;
	MojErr err = strName.assign(name);
	MojErrCheck(err);
	MojDbBerkeleyItem dbt;
	err = dbt.fromObject(strName);
	MojErrCheck(err);
	DB_SEQUENCE* seq = NULL;
	int dbErr = db_sequence_create(&seq, db->impl(), 0);
	MojBdbErrCheck(dbErr, _T("db_sequence_create"));
	MojAssert(seq);
	m_seq = seq;
	dbErr = m_seq->set_cachesize(m_seq, defs::MojSeqCacheSize);
	MojBdbErrCheck(dbErr, _T("seq->set_cachesize"));
	dbErr = m_seq->open(m_seq, NULL, dbt.impl(), defs::MojSeqOpenFlags);
	MojBdbErrCheck(dbErr, _T("seq->open"));

	// keep a reference to this seq
	err = db->engine()->addSeq(this);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleySeq::close()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err = MojErrNone;
	if (m_seq) {
		MojErr errClose = closeImpl();
		MojErrAccumulate(err, errClose);
		m_db->engine()->removeSeq(this);
		m_db = NULL;
	}
	return err;
}

MojErr MojDbBerkeleySeq::closeImpl()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	int dbErr = m_seq->close(m_seq, 0);
	m_seq = NULL;
	MojBdbErrCheck(dbErr, _T("seq->close"));

	return MojErrNone;
}

MojErr MojDbBerkeleySeq::get(MojInt64& valOut)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	db_seq_t id = 0;
	int dbErr = m_seq->get(m_seq, NULL, 1, &id, defs::MojSeqGetFlags);
	MojBdbErrCheck(dbErr, _T("seq->get"));
    valOut = (MojInt64) id;

	return MojErrNone;
}

MojDbBerkeleyTxn::MojDbBerkeleyTxn()
: m_engine(NULL),
  m_txn(NULL),
  m_updateSize(0)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);
}

MojDbBerkeleyTxn::~MojDbBerkeleyTxn()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	if (m_txn) {
		abort();
	}
}

MojErr MojDbBerkeleyTxn::begin(MojDbBerkeleyEngine* eng)
{
	MojAssert(!m_txn && eng && eng->env());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	DB_ENV* dbEnv = eng->env()->impl();
	DB_TXN* txn = NULL;
	int dbErr = dbEnv->txn_begin(dbEnv, NULL, &txn, defs::MojTxnBeginFlags);
	MojBdbErrCheck(dbErr, _T("env->txn_begin"));
	MojAssert(txn);
	m_txn = txn;
	m_engine = eng;

	return MojErrNone;
}

MojErr MojDbBerkeleyTxn::commitImpl()
{
	MojAssert(m_txn);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	int dbErr = m_txn->commit(m_txn, 0);
	m_txn = NULL;
	MojBdbErrCheck(dbErr, _T("txn->commit"));

	if (m_updateSize != 0) {
		MojErr err = m_engine->env()->postCommit(m_updateSize);
		MojErrCheck(err);
	}
	return MojErrNone;
}

bool MojDbBerkeleyTxn::isValid()
{
    return true;
}

MojErr MojDbBerkeleyTxn::abort()
{
	MojAssert(m_txn);
	MojLogTrace(MojDbBerkeleyEngine::s_log);
	MojLogWarning(MojDbBerkeleyEngine::s_log, _T("bdb: transaction aborted"));

	int dbErr = m_txn->abort(m_txn);
	m_txn = NULL;
	MojBdbErrCheck(dbErr, _T("txn->abort"));

	return MojErrNone;
}
