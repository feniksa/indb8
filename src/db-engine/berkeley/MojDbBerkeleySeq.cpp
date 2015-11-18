#include "db-engine/berkeley/MojDbBerkeleySeq.h"
#include "db-engine/berkeley/MojDbBerkeleyEngine.h"
#include "db-engine/berkeley/MojDbBerkeleyDatabase.h"
#include "db-engine/berkeley/MojDbBerkeleyItem.h"
#include "db-engine/berkeley/MojDbBerkeleyEnv.h"

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
