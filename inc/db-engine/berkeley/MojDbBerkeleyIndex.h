#pragma once

#include "core/MojCoreDefs.h"
#include "core/MojObject.h"
#include "db-engine/berkeley/MojDbBerkeleyDefs.h"
#include "db/MojDbStorageIndex.h"

class MojDbBerkeleyIndex : public MojDbStorageIndex
{
public:
	MojDbBerkeleyIndex();
	virtual ~MojDbBerkeleyIndex();

	MojErr open(const MojObject& id, MojDbBerkeleyDatabase* db, MojDbStorageTxn* txn);
	virtual MojErr close();
	virtual MojErr drop(MojDbStorageTxn* txn);
	virtual MojErr stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut);
	virtual MojErr insert(const MojDbKey& key, MojDbStorageTxn* txn);
	virtual MojErr del(const MojDbKey& key, MojDbStorageTxn* txn);
	virtual MojErr find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut);
	virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut);

private:
	bool isOpen() const { return m_primaryDb.get() != NULL; }

	MojObject m_id;
	MojRefCountedPtr<MojDbBerkeleyDatabase> m_primaryDb;
	MojRefCountedPtr<MojDbBerkeleyDatabase> m_db;
};
