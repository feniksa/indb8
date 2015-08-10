#pragma once

#include "core/MojCoreDefs.h"
#include "db/MojDbKey.h"
#include "db-engine/berkeley/MojDbBerkeleyDefs.h"
#include <db.h>

class MojDbBerkeleyDatabase;
class MojDbStorageTxn;

class MojDbBerkeleyCursor : public MojNoCopy
{
public:
	MojDbBerkeleyCursor();
	~MojDbBerkeleyCursor();

	MojErr open(MojDbBerkeleyDatabase* db, MojDbStorageTxn* txn, MojUInt32 flags);
	MojErr close();
	MojErr del();
	MojErr delPrefix(const MojDbKey& prefix);
	MojErr get(MojDbBerkeleyItem& key, MojDbBerkeleyItem& val, bool& foundOut, MojUInt32 flags);
	MojErr stats(MojSize& countOut, MojSize& sizeOut);
	MojErr statsPrefix(const MojDbKey& prefix, MojSize& countOut, MojSize& sizeOut);

	DBC* impl() { return m_dbc; }

private:
	DBC* m_dbc;
	MojDbStorageTxn* m_txn;
	MojSize m_recSize;
	MojSize m_warnCount;
};
