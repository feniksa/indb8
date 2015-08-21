#include "db-engine/berkeley/MojDbBerkeleyEnv.h"
#include "db-engine/berkeley/MojDbBerkeleyEngine.h"
#include "core/MojObject.h"

const MojChar* const MojDbBerkeleyEnv::LockFileName = _T("_lock");


MojDbBerkeleyEnv::MojDbBerkeleyEnv()
: m_env(NULL),
m_flags(defs::MojEnvOpenFlags),
m_logFlags(defs::MojLogFlags),
m_logFileSize(defs::MojEnvDefaultLogFileSize),
m_logRegionSize(defs::MojEnvDefaultLogRegionSize),
m_cacheSize(defs::MojEnvDefaultCacheSize),
m_maxLocks(defs::MojEnvDefaultMaxLocks),
m_maxLockers(defs::MojEnvDefaultMaxLockers),
m_checkpointMinKb(defs::MojEnvDefaultCheckpointMinKb),
m_compactStepSize(defs::MojEnvDefaultCompactStepSize)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojFlagSet(m_flags, DB_PRIVATE, defs::MojEnvDefaultPrivate);
	MojFlagSet(m_logFlags, DB_LOG_AUTO_REMOVE, defs::MojEnvDefaultLogAutoRemove);
}

MojDbBerkeleyEnv::~MojDbBerkeleyEnv()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err =  close();
	MojErrCatchAll(err);
}

MojErr MojDbBerkeleyEnv::openStorage(MojRefCountedPtr<MojDbStorageEngine>& storage)
{
	MojRefCountedPtr<MojDbBerkeleyEnv> thiz(this);
	storage.reset(new MojDbBerkeleyEngine(thiz));
	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::configure(const MojObject& conf)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	bool dbPrivate = false;
	if (!conf.get(_T("private"), dbPrivate))
		dbPrivate = defs::MojEnvDefaultPrivate;
	bool logAutoRemove = false;
	if (!conf.get(_T("logAutoRemove"), logAutoRemove))
		logAutoRemove = defs::MojEnvDefaultLogAutoRemove;
	bool found = false;
	MojString logDir;
	MojErr err = conf.get(_T("logDir"), logDir, found);
	MojErrCheck(err);
	MojUInt32 logFileSize = 0;
	err = conf.get(_T("logFileSize"), logFileSize, found);
	MojErrCheck(err);
	if (!found)
		logFileSize = defs::MojEnvDefaultLogFileSize;
	MojUInt32 logRegionSize = 0;
	err = conf.get(_T("logRegionSize"), logRegionSize, found);
	MojErrCheck(err);
	if (!found)
		logRegionSize = defs::MojEnvDefaultLogRegionSize;
	MojUInt32 cacheSize = 0;
	err = conf.get(_T("cacheSize"), cacheSize, found);
	MojErrCheck(err);
	if (!found)
		cacheSize = defs::MojEnvDefaultCacheSize;
	MojUInt32 maxLocks = 0;
	err = conf.get(_T("maxLocks"), maxLocks, found);
	MojErrCheck(err);
	if (!found)
		maxLocks = defs::MojEnvDefaultMaxLocks;
	MojUInt32 maxLockers = 0;
	err = conf.get(_T("maxLockers"), maxLockers, found);
	MojErrCheck(err);
	if (!found)
		maxLockers = defs::MojEnvDefaultMaxLockers;
	MojUInt32 minKb = 0;
	err = conf.get(_T("checkpointMinKb"), minKb, found);
	MojErrCheck(err);
	if (!found)
		minKb = defs::MojEnvDefaultCheckpointMinKb;
	MojUInt32 compactStepSize = 0;
	err = conf.get(_T("compactStepSize"), compactStepSize, found);
	MojErrCheck(err);
	if (!found)
		compactStepSize = defs::MojEnvDefaultCompactStepSize;

	MojFlagSet(m_flags, DB_PRIVATE, dbPrivate);
	MojFlagSet(m_logFlags, DB_LOG_AUTO_REMOVE, logAutoRemove);
	m_logDir = logDir;
	m_logFileSize = logFileSize;
	m_cacheSize = cacheSize;
	m_checkpointMinKb = minKb;
	m_maxLocks = maxLocks;
	m_maxLockers = maxLockers;
	m_compactStepSize = compactStepSize;

	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::open(const MojChar* dir)
{
	MojAssert(dir);
	MojAssert(!m_env);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	// lock env
	MojErr err = lockDir(dir);
	MojErrCheck(err);
	// create env
	DB_ENV* env = NULL;
	int dbErr = db_env_create(&env, 0);
	MojBdbErrCheck(dbErr, _T("db_env_create"));
	MojAssert(env);
	m_env = env;
	// set flags
	dbErr = m_env->set_flags(m_env, defs::MojEnvFlags, 1);
	MojBdbErrCheck(dbErr, _T("dbenv->log_set_flags"));
	// configure logging
	if (!m_logDir.empty()) {
		dbErr = m_env->set_lg_dir(m_env, m_logDir);
		MojBdbErrCheck(dbErr, _T("dbenv->set_lg_dir"));
		err = MojCreateDirIfNotPresent(m_logDir);
		MojErrCheck(err);
	}
	dbErr = m_env->log_set_config(m_env, m_logFlags, 1);
	MojBdbErrCheck(dbErr, _T("dbenv->log_set_config"));
	if (m_logFileSize > 0) {
		dbErr = m_env->set_lg_max(m_env, m_logFileSize);
		MojBdbErrCheck(dbErr, _T("dbenv->set_lg_max"));
	}
	if (m_logRegionSize > 0) {
		dbErr = m_env->set_lg_regionmax(m_env, m_logRegionSize);
		MojBdbErrCheck(dbErr, _T("dbenv->set_lg_regionmax"));
	}
	// set cache size
	dbErr = m_env->set_cachesize(m_env, 0, m_cacheSize, 0);
	MojBdbErrCheck(dbErr, _T("dbenv->set_cachesize"));
	// set max txns
	dbErr = m_env->set_tx_max(m_env, defs::MojTxnMax);
	MojBdbErrCheck(dbErr, _T("dbenv->set_tx_max"));
	// set deadlock policy
	dbErr = m_env->set_lk_detect(m_env, DB_LOCK_DEFAULT);
	MojBdbErrCheck(dbErr, _T("dbenv->set_lk_detect"));
	// set err/msg callbacks
	m_env->set_errcall(m_env, errcall);
	m_env->set_msgcall(m_env, msgcall);
	// set max lock counts
	dbErr = m_env->set_lk_max_locks(m_env, m_maxLocks);
	MojBdbErrCheck(dbErr, _T("dbenv->set_lk_max_locks"));
	dbErr = m_env->set_lk_max_objects(m_env, m_maxLocks);
	MojBdbErrCheck(dbErr, _T("dbenv->set_lk_max_objects"));
	dbErr = m_env->set_lk_max_lockers(m_env, m_maxLockers);
	MojBdbErrCheck(dbErr, _T("dbenv->set_lk_max_lockers"));
	// open env
	dbErr = m_env->open(m_env, dir, defs::MojEnvOpenFlags, defs::MojDbFileMode);
	MojBdbErrCheck(dbErr, _T("dbenv->open"));

	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::close()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);
	MojErr err = MojErrNone;
	MojErr errClose = MojErrNone;

	if (m_env) {
		// checkpoint before close
		errClose = checkpoint(0);
		MojErrAccumulate(err, errClose);
		int dbErr = m_env->close(m_env, 0);
		MojBdbErrAccumulate(err, dbErr, _T("dbenv->close"));
		m_env = NULL;
	}
	errClose = unlockDir();
	MojErrAccumulate(err, errClose);

	return err;
}

MojErr MojDbBerkeleyEnv::postCommit(MojSize updateSize)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	// every N updates, check to see if we have enough log data
	// to justify a checkpoint
	if ((m_updateSize += (MojInt32) updateSize) >= (m_checkpointMinKb * 1024)) {
		MojErr err = tryCheckpoint(0);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::checkpoint(MojUInt32 minKB)
{
	MojAssert(m_env);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojThreadGuard guard(m_checkpointMutex);
	MojErr err = checkpointImpl(minKB);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::tryCheckpoint(MojUInt32 minKB)
{
	MojAssert(m_env);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojThreadGuard guard(m_checkpointMutex, false);
	if (guard.tryLock()) {
		MojErr err = checkpointImpl(minKB);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::checkpointImpl(MojUInt32 minKB)
{
	MojAssert(m_env);
	MojAssertMutexLocked(m_checkpointMutex);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	int dbErr = m_env->txn_checkpoint(m_env, minKB, 0, 0);
	MojBdbErrCheck(dbErr, _T("dbenv->txn_checkpoint"));
	// this is an approximation, so it's ok if we don't count a few
	m_updateSize = 0;
	if (MojFlagGet(m_logFlags, DB_LOG_AUTO_REMOVE)) {
		MojErr err = purgeLogs();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::purgeLogs()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	// get list of archivable logs
	char** logs = NULL;
	int dbErr = m_env->log_archive(m_env, &logs, defs::MojLogArchiveFlags);
	MojBdbErrCheck(dbErr, _T("dbenv->log_archive"));
	// delete them all
	if (logs) {
		MojAutoFreePtr<char*> ap(logs);
		for (char** curLog = logs; *curLog; ++curLog) {
			MojErr err = MojUnlink(*curLog);
			MojErrCatch(err, MojErrNotFound);
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::lockDir(const MojChar* path)
{
	MojAssert(path);

	MojErr err = MojCreateDirIfNotPresent(path);
	MojErrCheck(err);
	err = m_lockFileName.format(_T("%s/%s"), path, LockFileName);
	MojErrCheck(err);
	err = m_lockFile.open(m_lockFileName, MOJ_O_RDWR | MOJ_O_CREAT, MOJ_S_IRUSR | MOJ_S_IWUSR);
	MojErrCheck(err);
	err = m_lockFile.lock(LOCK_EX | LOCK_NB);
	MojErrCatch(err, MojErrWouldBlock) {
		(void) m_lockFile.close();
		MojErrThrowMsg(MojErrLocked, _T("Database at '%s' locked by another instance."), path);
	}
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::unlockDir()
{
	MojErr err = MojErrNone;
	if (m_lockFile.open()) {
		// unlink before we close to ensure that we hold
		// the lock to the bitter end
		MojErr errClose = MojUnlink(m_lockFileName);
		MojErrAccumulate(err, errClose);
		errClose = m_lockFile.close();
		MojErrAccumulate(err, errClose);
	}
	return err;
}

MojErr MojDbBerkeleyEnv::translateErr(int dbErr)
{
	switch (dbErr) {
		case DB_LOCK_DEADLOCK:
			return MojErrDbDeadlock;
		case DB_NOTFOUND:
			return MojErrNotFound;
		case DB_RUNRECOVERY:
			return MojErrDbFatal;
		case DB_VERIFY_BAD:
			return MojErrDbVerificationFailed;
	}
	return (MojErr) dbErr;
}

void MojDbBerkeleyEnv::errcall(const DB_ENV *dbenv, const char *errpfx, const char *msg)
{
	MojLogError(MojDbBerkeleyEngine::s_log, "bdb: %s\n", msg);
}

void MojDbBerkeleyEnv::msgcall(const DB_ENV *dbenv, const char *msg)
{
	MojLogError(MojDbBerkeleyEngine::s_log, "bdb: %s\n", msg);
}
