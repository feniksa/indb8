#include "db-engine/berkeley/MojDbBerkeleyTxn.h"
#include "db-engine/berkeley/MojDbBerkeleyEngine.h"
#include "db-engine/berkeley/MojDbBerkeleyEnv.h"

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
