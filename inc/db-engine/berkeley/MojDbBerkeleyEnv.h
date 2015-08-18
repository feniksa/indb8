#pragma once

#include "core/MojCoreDefs.h"
#include "core/MojFile.h"
#include "core/MojThread.h"
#include "core/MojString.h"
#include "db/MojDbEnv.h"
#include "db-engine/berkeley/MojDbBerkeleyDefs.h"
#include <db.h>

class MojDbBerkeleyEnv : public MojDbEnv
{
public:
	static MojLogger s_log;

	MojDbBerkeleyEnv();
	~MojDbBerkeleyEnv();

	MojErr configure(const MojObject& conf);
	MojErr open(const MojChar* path);
	MojErr close();
	MojErr postCommit(MojSize updateSize);
	MojErr checkpoint(MojUInt32 minKB);

	const MojUInt32 compactStepSize(void) const { return m_compactStepSize; }
	DB_ENV* impl() { return m_env; }
	static MojErr translateErr(int dbErr);

private:
	static const MojChar* const LockFileName;

	MojErr tryCheckpoint(MojUInt32 minKB);
	MojErr checkpointImpl(MojUInt32 minKB);
	MojErr create(const MojChar* dir);
	MojErr purgeLogs();
	MojErr lockDir(const MojChar* path);
	MojErr unlockDir();

	static void errcall(const DB_ENV *dbenv, const char *errpfx, const char *msg);
	static void msgcall(const DB_ENV *dbenv, const char *msg);

	MojAtomicInt m_updateSize;
	MojThreadMutex m_checkpointMutex;
	MojString m_lockFileName;
	MojFile m_lockFile;
	DB_ENV* m_env;

	MojString m_logDir;
	MojUInt32 m_flags;
	MojUInt32 m_logFlags;
	MojUInt32 m_logFileSize;
	MojUInt32 m_logRegionSize;
	MojUInt32 m_cacheSize;
	MojUInt32 m_maxLocks;
	MojUInt32 m_maxLockers;
	MojUInt32 m_checkpointMinKb;
	MojUInt32 m_compactStepSize;
};
