#include "db-engine/berkeley/MojDbBerkeleyDefs.h"
#include <sys/statvfs.h>
#include <db.h>

// DB
const MojUInt32 defs::MojDbOpenFlags = DB_THREAD /*| DB_MULTIVERSION*/; // NOTE: MULTIVERSION disabled due to leaked transactions
const MojUInt32 defs::MojDbGetFlags = DB_READ_COMMITTED;
// LOG
const MojUInt32 defs::MojLogFlags = 0;
const MojUInt32 defs::MojLogArchiveFlags = DB_ARCH_ABS;

// SEQ
const MojUInt32 defs::MojSeqOpenFlags = DB_CREATE | DB_THREAD;
const MojUInt32 defs::MojSeqGetFlags = DB_TXN_NOSYNC;
const MojUInt32 defs::MojSeqCacheSize = 25;

// GENERAL
const int defs::MojDbFileMode = MOJ_S_IRUSR | MOJ_S_IWUSR;

// ENV
const MojUInt32 defs::MojEnvFlags = DB_AUTO_COMMIT | DB_NOMMAP;
const MojUInt32 defs::MojEnvOpenFlags = DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN | DB_RECOVER | DB_THREAD | DB_PRIVATE | DB_DIRECT_DB;
const bool defs::MojEnvDefaultPrivate = true;
const bool defs::MojEnvDefaultLogAutoRemove = true;
const MojUInt32 defs::MojEnvDefaultLogFileSize = 1024 * 1024 * 2; // 2MB
const MojUInt32 defs::MojEnvDefaultLogRegionSize = 1024 * 512; // 512 KB
const MojUInt32 defs::MojEnvDefaultCacheSize = 1024 * 1024 * 2; // 2MB
const MojUInt32 defs::MojEnvDefaultMaxLocks = 8000; // max db size / page size
const MojUInt32 defs::MojEnvDefaultMaxLockers = 1000 * 2; // twice the default
const MojUInt32 defs::MojEnvDefaultCheckpointMinKb = 512; // 1MB
const MojUInt32 defs::MojEnvDefaultCheckpointMinMinutes = 0;
const MojUInt32 defs::MojEnvDefaultCompactStepSize = 25000;
const MojChar* const defs::MojEnvSeqDbName = _T("sequences.db");
const MojChar* const defs::MojEnvIndexDbName = _T("indexes.db");

// TXN
const MojUInt32 defs::MojTxnBeginFlags = DB_READ_COMMITTED;
const MojUInt32 defs::MojTxnMax = 100; // need up to one per cache page for mvcc
