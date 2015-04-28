#pragma once

#include "db/MojDbStorageCollection.h"
#include "core/MojVector.h"

class MojDbStorageIndex : public MojDbStorageCollection
{
public:
	typedef MojVector<MojByte> ByteVec;

	virtual ~MojDbStorageIndex() {}
	virtual MojErr insert(const MojDbKey& key, MojDbStorageTxn* txn) = 0;
	virtual MojErr del(const MojDbKey& key, MojDbStorageTxn* txn) = 0;
};
