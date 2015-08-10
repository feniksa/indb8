#include "db-engine/berkeley/MojDbBerkeleyIndex.h"
#include "db-engine/berkeley/MojDbBerkeleyDatabase.h"
#include "db-engine/berkeley/MojDbBerkeleyEngine.h"
#include "db-engine/berkeley/MojDbBerkeleyCursor.h"
#include "db-engine/berkeley/MojDbBerkeleyItem.h"
#include "db-engine/berkeley/MojDbBerkeleyQuery.h"

MojDbBerkeleyIndex::MojDbBerkeleyIndex()
: m_primaryDb(NULL)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);
}

MojDbBerkeleyIndex::~MojDbBerkeleyIndex()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err =  close();
	MojErrCatchAll(err);
}

MojErr MojDbBerkeleyIndex::open(const MojObject& id, MojDbBerkeleyDatabase* db, MojDbStorageTxn* txn)
{
	MojAssert(db && db->engine());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	m_id = id;
	m_db.reset(db->engine()->indexDb());
	m_primaryDb.reset(db);

	return MojErrNone;
}

MojErr MojDbBerkeleyIndex::close()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	m_db.reset();
	m_primaryDb.reset();

	return MojErrNone;
}

MojErr MojDbBerkeleyIndex::drop(MojDbStorageTxn* txn)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojDbBerkeleyCursor cursor;
	MojErr err = cursor.open(m_db.get(), txn, 0);
	MojErrCheck(err);
	MojDbKey prefix;
	err = prefix.assign(m_id);
	MojErrCheck(err);
	err = cursor.delPrefix(prefix);
	MojErrCheck(err);
	err = cursor.close();
	MojErrCheck(err);

	return err;
}

MojErr MojDbBerkeleyIndex::stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojDbBerkeleyCursor cursor;
	MojErr err = cursor.open(m_db.get(), txn, 0);
	MojErrCheck(err);
	MojDbKey prefix;
	err = prefix.assign(m_id);
	MojErrCheck(err);
	err = cursor.statsPrefix(prefix, countOut, sizeOut);
	MojErrCheck(err);
	err = cursor.close();
	MojErrCheck(err);

	return err;
}

MojErr MojDbBerkeleyIndex::insert(const MojDbKey& key, MojDbStorageTxn* txn)
{
	MojAssert(txn);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojDbBerkeleyItem keyItem;
	keyItem.fromBytesNoCopy(key.data(), key.size());
	MojDbBerkeleyItem valItem;  // ???
	MojErr err = m_db->put(keyItem, valItem, txn, true);
	#ifdef MOJ_DEBUG
	char s[1024];
	size_t size1 = keyItem.size();
	size_t size2 = valItem.size();
	MojErr err2 = MojByteArrayToHex(keyItem.data(), size1, s);
	MojErrCheck(err2);
	if (size1 > 16)	// if the object-id is in key
		strncat(s, (char *)(keyItem.data()) + (size1 - 17), 16);
	MojLogDebug(MojDbBerkeleyEngine::s_log, _T("bdbindexinsert: %s; keylen: %zu, key: %s ; vallen = %zu; err = %d\n"),
				m_db->m_name.data(), size1, s, size2, err);
	#endif
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyIndex::del(const MojDbKey& key, MojDbStorageTxn* txn)
{
	MojAssert(txn);
	MojAssert(isOpen());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojDbBerkeleyItem keyItem;
	keyItem.fromBytesNoCopy(key.data(), key.size());

	bool found = false;
	MojErr err = m_db->del(keyItem, found, txn);

	#ifdef MOJ_DEBUG
	char s[1024];
	size_t size1 = keyItem.size();
	MojErr err2 = MojByteArrayToHex(keyItem.data(), size1, s);
	MojErrCheck(err2);
	if (size1 > 16)	// if the object-id is in key
		strncat(s, (char *)(keyItem.data()) + (size1 - 17), 16);
	MojLogDebug(MojDbBerkeleyEngine::s_log, _T("bdbindexdel: %s; keylen: %zu, key: %s ; err = %d\n"), m_db->m_name.data(), size1, s, err);
	if (!found)
		MojLogWarning(MojDbBerkeleyEngine::s_log, _T("bdbindexdel_warn: not found: %s \n"), s);
	#endif

	MojErrCheck(err);
	if (!found) {
		//MojErrThrow(MojErrDbInconsistentIndex);   // fix this to work around to deal with out of sync indexes
		MojErrThrow(MojErrInternalIndexOnDel);
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyIndex::find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut)
{
	MojAssert(isOpen());
	MojAssert(plan.get() && txn);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojRefCountedPtr<MojDbBerkeleyQuery> storageQuery(new MojDbBerkeleyQuery());
	MojAllocCheck(storageQuery.get());
	MojErr err = storageQuery->open(m_db.get(), m_primaryDb.get(), plan, txn);
	MojErrCheck(err);
	queryOut = storageQuery;

	return MojErrNone;
}

MojErr MojDbBerkeleyIndex::beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	return m_primaryDb->beginTxn(txnOut);
}
