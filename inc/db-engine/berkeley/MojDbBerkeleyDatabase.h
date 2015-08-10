#pragma once

#include "core/MojCoreDefs.h"
#include "core/MojString.h"
#include "db/MojDbStorageDatabase.h"
#include "db-engine/berkeley/MojDbBerkeleyDefs.h"
#include <db.h>

class MojDbBerkeleyDatabase : public MojDbStorageDatabase
{
public:
	MojDbBerkeleyDatabase();
	~MojDbBerkeleyDatabase();

	MojErr open(const MojChar* dbName, MojDbBerkeleyEngine* env, bool& createdOut, MojDbStorageTxn* txn);
	virtual MojErr close();
	virtual MojErr drop(MojDbStorageTxn* txn);
	virtual MojErr stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut);
	virtual MojErr insert(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn);
	virtual MojErr update(const MojObject& id, MojBuffer& val, MojDbStorageItem* oldVal, MojDbStorageTxn* txn);
	virtual MojErr del(const MojObject& id, MojDbStorageTxn* txn, bool& foundOut);
	virtual MojErr get(const MojObject& id, MojDbStorageTxn* txn, bool forUpdate, MojRefCountedPtr<MojDbStorageItem>& itemOut);
	virtual MojErr find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut);
	virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut);
	virtual MojErr openIndex(const MojObject& id, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageIndex>& indexOut);

	// work-around - specific for BDB only
	virtual MojErr mutexStats(int* total_mutexes, int* mutexes_free, int* mutexes_used, int* mutexes_used_highwater, int* mutex_regionsize);

	MojErr put(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn, bool updateIdQuota);
	MojErr put(MojDbBerkeleyItem& key, MojDbBerkeleyItem& val, MojDbStorageTxn* txn, bool updateIdQuota);
	MojErr del(MojDbBerkeleyItem& key, bool& foundOut, MojDbStorageTxn* txn);
	MojErr get(MojDbBerkeleyItem& key, MojDbStorageTxn* txn, bool forUpdate, MojDbBerkeleyItem& valOut, bool& foundOut);

	DB* impl() { return m_db; }
	MojDbBerkeleyEngine* engine() { return m_engine; }

private:
	friend class MojDbBerkeleyEngine;
	friend class MojDbBerkeleyIndex;

	MojErr verify();
	MojErr closeImpl();
	void postUpdate(MojDbStorageTxn* txn, MojSize updateSize);

	DB* m_db;
	MojDbBerkeleyEngine* m_engine;
	MojString m_file;
	MojString m_name;
	MojVector<MojString> m_primaryProps;
};
