#include "db-engine/berkeley/MojDbBerkeleyDatabase.h"
#include "db-engine/berkeley/MojDbBerkeleyEngine.h"
#include "db-engine/berkeley/MojDbBerkeleyEnv.h"
#include "db-engine/berkeley/MojDbBerkeleyTxn.h"
#include "db-engine/berkeley/MojDbBerkeleyCursor.h"
#include "db-engine/berkeley/MojDbBerkeleyItem.h"
#include "db-engine/berkeley/MojDbBerkeleyQuery.h"
#include "db-engine/berkeley/MojDbBerkeleyIndex.h"
#include "db/MojDb.h"

MojDbBerkeleyDatabase::MojDbBerkeleyDatabase()
: m_db(NULL)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);
}

MojDbBerkeleyDatabase::~MojDbBerkeleyDatabase()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err =  close();
	MojErrCatchAll(err);
}

MojErr MojDbBerkeleyDatabase::open(const MojChar* dbName, MojDbBerkeleyEngine* eng, bool& createdOut, MojDbStorageTxn* txn)
{
	MojAssert(dbName && eng);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	// save eng, name and file
	createdOut = false;
	m_engine = eng;
	MojErr err = m_name.assign(dbName);
	MojErrCheck(err);
	const MojString& engPath = eng->path();
	if (engPath.empty()) {
		err = m_file.assign(dbName);
		MojErrCheck(err);
	} else {
		err = m_file.format(_T("%s/%s"), engPath.data(), dbName);
		MojErrCheck(err);
	}

	// create and open db
	DB* db = NULL;
	int dbErr = db_create(&db, eng->env()->impl(), 0);
	MojBdbErrCheck(dbErr, _T("db_create"));
	MojAssert(db);
	m_db = db;

	DB_TXN* dbTxn = MojBdbTxnFromStorageTxn(txn);
	MojUInt32 flags = defs::MojDbOpenFlags;
	if (!dbTxn)
		flags |= DB_AUTO_COMMIT;
	// try once without the DB_CREATE flag
	dbErr = m_db->open(m_db, dbTxn, m_file, dbName, DB_BTREE, flags, defs::MojDbFileMode);
	if (dbErr == MojErrNotFound) {
		// if open failed, we know that we had to create the db
		createdOut = true;
		flags |= DB_CREATE;
		dbErr = m_db->open(m_db, dbTxn, m_file, dbName, DB_BTREE, flags, defs::MojDbFileMode);
	}
	MojBdbErrCheck(dbErr, _T("db->open"));
	// set up prop-vec for primary key queries
	MojString idStr;
	err = idStr.assign(MojDb::IdKey);
	MojErrCheck(err);
	err = m_primaryProps.push(idStr);
	MojErrCheck(err);

	//keep a reference to this database
	err = eng->addDatabase(this);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::close()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err = MojErrNone;
	if (m_db) {
		MojErr errClose = closeImpl();
		MojErrAccumulate(err, errClose);
		m_primaryProps.clear();
		engine()->removeDatabase(this);
	}
	return err;
}

MojErr MojDbBerkeleyDatabase::drop(MojDbStorageTxn* txn)
{
	MojAssert(m_engine && !m_db);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	DB_ENV* env = m_engine->env()->impl();
	DB_TXN* dbTxn = MojBdbTxnFromStorageTxn(txn);
	MojUInt32 flags = dbTxn ? 0 : DB_AUTO_COMMIT;
	int dbErr = env->dbremove(env, dbTxn, m_file, NULL, flags);
	MojBdbErrCheck(dbErr, _T("env->dbremove"));

	return MojErrNone;
}


MojErr MojDbBerkeleyDatabase::mutexStats(int * total_mutexes, int * mutexes_free, int * mutexes_used,
										 int * mutexes_used_highwater, int * mutexes_regionsize)
{
	MojAssert(m_engine);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	DB_ENV* env = m_engine->env()->impl();
	DB_MUTEX_STAT * statp = NULL;
	int dbErr = env->mutex_stat(env, &statp, 0);
	MojBdbErrCheck(dbErr, _T("env->mutex_stat"));

	if (total_mutexes)
		*total_mutexes = statp->st_mutex_cnt;
	if (mutexes_free)
		*mutexes_free = statp->st_mutex_free;
	if (mutexes_used)
		*mutexes_used = statp->st_mutex_inuse;
	if (mutexes_used_highwater)
		*mutexes_used_highwater = statp->st_mutex_inuse_max;
	if (mutexes_regionsize)
		*mutexes_regionsize = (int)(statp->st_regsize);

	free(statp);

	return MojErrNone;
}


MojErr MojDbBerkeleyDatabase::stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojDbBerkeleyCursor cursor;
	MojErr err = cursor.open(this, txn, 0);
	MojErrCheck(err);
	err = cursor.stats(countOut, sizeOut);
	MojErrCheck(err);
	err = cursor.close();
	MojErrCheck(err);

	return err;
}

MojErr MojDbBerkeleyDatabase::insert(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn)
{
	MojAssert(txn);

	MojErr err = put(id, val, txn, true);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::update(const MojObject& id, MojBuffer& val, MojDbStorageItem* oldVal, MojDbStorageTxn* txn)
{
	MojAssert(oldVal && txn);

	MojErr err = txn->offsetQuota(-(MojInt64) oldVal->size());
	MojErrCheck(err);
	err = put(id, val, txn, false);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::del(const MojObject& id, MojDbStorageTxn* txn, bool& foundOut)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojDbBerkeleyItem idItem;
	MojErr err = idItem.fromObject(id);
	MojErrCheck(err);
	err = del(idItem, foundOut, txn);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::get(const MojObject& id, MojDbStorageTxn* txn, bool forUpdate, MojRefCountedPtr<MojDbStorageItem>& itemOut)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	itemOut.reset();
	MojDbBerkeleyItem idItem;
	MojErr err = idItem.fromObject(id);
	MojErrCheck(err);
	MojRefCountedPtr<MojDbBerkeleyItem> valItem(new MojDbBerkeleyItem);
	MojAllocCheck(valItem.get());
	bool found = false;
	err = get(idItem, txn, forUpdate, *valItem, found);
	MojErrCheck(err);
	if (found) {
		valItem->id(id);
		itemOut = valItem;
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut)
{
	MojAssert(m_db);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojRefCountedPtr<MojDbBerkeleyQuery> storageQuery(new MojDbBerkeleyQuery);
	MojAllocCheck(storageQuery.get());
	MojErr err = storageQuery->open(this, NULL, plan, txn);
	MojErrCheck(err);
	queryOut = storageQuery;

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::put(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn, bool updateIdQuota)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojDbBerkeleyItem idItem;
	MojErr err = idItem.fromObject(id);
	MojErrCheck(err);
	MojDbBerkeleyItem valItem;
	err = valItem.fromBuffer(val);
	MojErrCheck(err);
	err = put(idItem, valItem, txn, updateIdQuota);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::put(MojDbBerkeleyItem& key, MojDbBerkeleyItem& val, MojDbStorageTxn* txn, bool updateIdQuota)
{
	MojAssert(m_db && txn);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojInt64 quotaOffset = val.size();
	if (updateIdQuota)
		quotaOffset += key.size();
	MojErr err = txn->offsetQuota(quotaOffset);
	MojErrCheck(err);
	DB_TXN* dbTxn = MojBdbTxnFromStorageTxn(txn);
	int dbErr = m_db->put(m_db, dbTxn, key.impl(), val.impl(), 0);
	#if defined(MOJ_DEBUG)
	char s[1024];
	size_t size1 = key.size();
	size_t size2 = val.size();
	MojErr err2 = MojByteArrayToHex(key.data(), size1, s);
	MojErrCheck(err2);
	if (size1 > 16)	// if the object-id is in key
		strncat(s, (char *)(key.data()) + (size1 - 17), 16);
	MojLogDebug(MojDbBerkeleyEngine::s_log, _T("bdbput: %s; keylen: %zu, key: %s ; vallen = %zu; err = %d\n"),
				this->m_name.data(), size1, s, size2, err);
	#endif
	MojBdbErrCheck(dbErr, _T("db->put"));
	postUpdate(txn, key.size() + val.size());

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::get(MojDbBerkeleyItem& key, MojDbStorageTxn* txn, bool forUpdate,
								  MojDbBerkeleyItem& valOut, bool& foundOut)
{
	MojAssert(m_db);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	foundOut = false;
	// acquire a write lock if we are going to do an update
	int flags = defs::MojDbGetFlags;
	if (forUpdate)
		flags |= DB_RMW;
	DB_TXN* dbTxn = MojBdbTxnFromStorageTxn(txn);
	int dbErr = m_db->get(m_db, dbTxn, key.impl(), valOut.impl(), 0);
	if (dbErr != DB_NOTFOUND) {
		MojBdbErrCheck(dbErr, _T("db->get"));
		foundOut = true;
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
{
	MojAssert(!txnOut.get());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err = m_engine->beginTxn(txnOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::openIndex(const MojObject& id, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageIndex>& indexOut)
{
	MojAssert(!indexOut.get());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojRefCountedPtr<MojDbBerkeleyIndex> index(new MojDbBerkeleyIndex());
	MojAllocCheck(index.get());
	MojErr err = index->open(id, this, txn);
	MojErrCheck(err);
	indexOut = index;

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::del(MojDbBerkeleyItem& key, bool& foundOut, MojDbStorageTxn* txn)
{

	MojAssert(m_db);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	foundOut = false;
	MojErr err = txn->offsetQuota(-(MojInt64) key.size());
	MojErrCheck(err);
	DB_TXN* dbTxn = MojBdbTxnFromStorageTxn(txn);
	int dbErr = m_db->del(m_db, dbTxn, key.impl(), 0);

	#if defined(MOJ_DEBUG)
	char s[1024];	// big enough for any key
	size_t size = key.size();
	MojErr err2 = MojByteArrayToHex(key.data(), size, s);
	MojErrCheck(err2);
	if (size > 16)	// if the object-id is in key
		strncat(s, (char *)(key.data()) + (size - 17), 16);
	MojLogDebug(MojDbBerkeleyEngine::s_log, _T("bdbdel: %s; keylen: %zu, key= %s; err= %d \n"), this->m_name.data(), size, s, dbErr);
	#endif

	if (dbErr != DB_NOTFOUND) {
		MojBdbErrCheck(dbErr, _T("db->get"));
		foundOut = true;
	}
	postUpdate(txn, key.size());

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::verify()
{
	DB* db = NULL;
	int dbErr = db_create(&db, m_engine->env()->impl(), 0);
	MojBdbErrCheck(dbErr, _T("db_create"));
	MojAssert(db);
	dbErr = db->verify(db, m_file, m_name, NULL, 0);
	MojErrCatch(dbErr, MojErrNotFound);
	MojBdbErrCheck(dbErr, _T("db_verify"));

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::closeImpl()
{
	int dbErr = m_db->close(m_db, 0);
	m_db = NULL;
	MojBdbErrCheck(dbErr, _T("db->close"));

	return MojErrNone;
}

void MojDbBerkeleyDatabase::postUpdate(MojDbStorageTxn* txn, MojSize size)
{
	if (txn) {
		static_cast<MojDbBerkeleyTxn*>(txn)->didUpdate(size);
	}
}
