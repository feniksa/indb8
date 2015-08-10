#include "db-engine/berkeley/MojDbBerkeleyCursor.h"
#include "db-engine/berkeley/MojDbBerkeleyDatabase.h"
#include "db-engine/berkeley/MojDbBerkeleyTxn.h"
#include "db-engine/berkeley/MojDbBerkeleyEnv.h"
#include "db-engine/berkeley/MojDbBerkeleyItem.h"

MojDbBerkeleyCursor::MojDbBerkeleyCursor()
: m_dbc(NULL),
m_txn(NULL),
m_recSize(0)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);
}

MojDbBerkeleyCursor::~MojDbBerkeleyCursor()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err =  close();
	MojErrCatchAll(err);
}

MojErr MojDbBerkeleyCursor::open(MojDbBerkeleyDatabase* db, MojDbStorageTxn* txn, MojUInt32 flags)
{
	MojAssert(db && txn);
	MojAssert(!m_dbc);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	DB* bdb = db->impl();
	DB_TXN* dbTxn = MojBdbTxnFromStorageTxn(txn);
	DBC* dbc = NULL;
	int dbErr = bdb->cursor(bdb, dbTxn, &dbc, flags);
	MojBdbErrCheck(dbErr, _T("db->cursor"));
	MojAssert(dbc);
	m_dbc = dbc;
	m_txn = txn;
	m_warnCount = 0;

	return MojErrNone;
}

MojErr MojDbBerkeleyCursor::close()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err = MojErrNone;
	if (m_dbc) {
		int dbErr = m_dbc->close(m_dbc);
		MojBdbErrAccumulate(err, dbErr, _T("dbc->close"));
		m_dbc = NULL;
		m_txn = NULL;
	}
	return err;
}

MojErr MojDbBerkeleyCursor::del()
{
	MojAssert(m_dbc);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	int dbErr = m_dbc->del(m_dbc, 0);
	MojBdbErrCheck(dbErr, _T("dbc->del"));
	MojErr err = m_txn->offsetQuota(-(MojInt64) m_recSize);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyCursor::delPrefix(const MojDbKey& prefix)
{
	MojDbBerkeleyItem val;
	MojDbBerkeleyItem key;
	MojErr err = key.fromBytes(prefix.data(), prefix.size());
	MojErrCheck(err);

	bool found = false;
	err = get(key, val, found, DB_SET_RANGE);
	MojErrCheck(err);
	while (found && key.hasPrefix(prefix)) {
		err = del();
		MojErrCheck(err);
		err = get(key, val, found, DB_NEXT);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyCursor::get(MojDbBerkeleyItem& key, MojDbBerkeleyItem& val, bool& foundOut, MojUInt32 flags)
{
	MojAssert(m_dbc);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	foundOut = false;
	int dbErr = m_dbc->get(m_dbc, key.impl(), val.impl(), flags);
	if (dbErr != DB_NOTFOUND) {
		MojBdbErrCheck(dbErr, _T("dbc->get"));
		foundOut = true;
		m_recSize = key.size() + val.size();
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyCursor::stats(MojSize& countOut, MojSize& sizeOut)
{

	MojErr err = statsPrefix(MojDbKey(), countOut, sizeOut);
	MojLogDebug(MojDbBerkeleyEngine::s_log, _T("bdbcursor_stats: count: %d, size: %d, err: %d"), (int)countOut, (int)sizeOut, (int)err);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyCursor::statsPrefix(const MojDbKey& prefix, MojSize& countOut, MojSize& sizeOut)
{
	countOut = 0;
	sizeOut = 0;
	m_warnCount = 0;	// debug
	MojDbBerkeleyItem val;
	MojDbBerkeleyItem key;
	MojErr err = key.fromBytes(prefix.data(), prefix.size());
	MojErrCheck(err);

	MojSize count = 0;
	MojSize size = 0;
	bool found = false;
	err = get(key, val, found, DB_SET_RANGE);
	MojErrCheck(err);
	while (found && key.hasPrefix(prefix)) {
		++count;
		// debug code for verifying index keys
		size += key.size() + val.size();
		err = get(key, val, found, DB_NEXT);
		MojErrCheck(err);
	}
	sizeOut = size;
	countOut = count;

	return MojErrNone;
}
