#pragma once

#include "core/MojCoreDefs.h"

class MojDbBerkeleyCursor;
class MojDbBerkeleyDatabase;
class MojDbBerkeleyEngine;
class MojDbBerkeleyEnv;
class MojDbBerkeleyIndex;
class MojDbBerkeleyItem;
class MojDbBerkeleyQuery;
class MojDbBerkeleySeq;
class MojDbBerkeleyTxn;

#define MojBdbErrCheck(E, FNAME)				if (E) MojErrThrowMsg(MojDbBerkeleyEnv::translateErr(E), _T("bdb: %s - %s"), FNAME, db_strerror(E))
#define MojBdbErrAccumulate(EACC, E, FNAME)		if (E) MojErrAccumulate(EACC, MojDbBerkeleyEnv::translateErr(E))
#define MojBdbTxnFromStorageTxn(TXN)			((TXN) ? static_cast<MojDbBerkeleyTxn*>(TXN)->impl() : NULL)

struct defs
{
// DB
static const MojUInt32 MojDbOpenFlags;
static const MojUInt32 MojDbGetFlags;

// SEQ
static const MojUInt32 MojSeqOpenFlags;
static const MojUInt32 MojSeqGetFlags;
static const MojUInt32 MojSeqCacheSize;

// GENERAL
static const int MojDbFileMode;

// ENV
static const MojUInt32 MojEnvFlags;
static const MojUInt32 MojEnvOpenFlags;
static const bool MojEnvDefaultPrivate;
static const bool MojEnvDefaultLogAutoRemove;
static const MojUInt32 MojEnvDefaultLogFileSize;
static const MojUInt32 MojEnvDefaultLogRegionSize;
static const MojUInt32 MojEnvDefaultCacheSize;
static const MojUInt32 MojEnvDefaultMaxLocks;
static const MojUInt32 MojEnvDefaultMaxLockers;
static const MojUInt32 MojEnvDefaultCheckpointMinKb;
static const MojUInt32 MojEnvDefaultCheckpointMinMinutes;
static const MojUInt32 MojEnvDefaultCompactStepSize;
static const MojChar* const MojEnvSeqDbName;
static const MojChar* const MojEnvIndexDbName;

// TXN
static const MojUInt32 MojTxnBeginFlags;
static const MojUInt32 MojTxnMax;

// LOG
static const MojUInt32 MojLogFlags;
static const MojUInt32 MojLogArchiveFlags;

};
