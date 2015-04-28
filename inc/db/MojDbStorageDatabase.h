#pragma once

#include "db/MojDbStorageCollection.h"

class MojDbStorageDatabase : public MojDbStorageCollection
{
public:
	virtual ~MojDbStorageDatabase() {}
	virtual MojErr insert(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn) = 0;
	virtual MojErr update(const MojObject& id, MojBuffer& val, MojDbStorageItem* oldVal, MojDbStorageTxn* txn) = 0;
	virtual MojErr del(const MojObject& id, MojDbStorageTxn* txn, bool& foundOut) = 0;
	virtual MojErr get(const MojObject& id, MojDbStorageTxn* txn, bool forUpdate, MojRefCountedPtr<MojDbStorageItem>& itemOut) = 0;
	virtual MojErr openIndex(const MojObject& id, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageIndex>& indexOut) = 0;
	//hack:
	virtual MojErr mutexStats(int* total_mutexes, int* mutexes_free, int* mutexes_used, int* mutexes_used_highwater, int* mutexes_regionsize)
	{ if (total_mutexes) *total_mutexes = 0;
		if (mutexes_free) *mutexes_free = 0;
		if (mutexes_used) *mutexes_used = 0;
		if (mutexes_used_highwater) *mutexes_used_highwater = 0;
		if (mutexes_regionsize) *mutexes_regionsize = 0;
		return MojErrNone;
	}
};
