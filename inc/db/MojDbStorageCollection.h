#pragma once

#include "db/MojDbDefs.h"
#include "core/MojRefCount.h"

class MojDbStorageCollection : public MojRefCounted
{
public:
	virtual ~MojDbStorageCollection() {}
	virtual MojErr close() = 0;
	virtual MojErr drop(MojDbStorageTxn* txn) = 0;
	virtual MojErr stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut) = 0;
	virtual MojErr find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut) = 0;
	virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut) = 0;
};
