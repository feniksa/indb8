#pragma once

#include "core/MojCoreDefs.h"
#include "core/MojRefCount.h"

class MojDbStorageSeq : public MojRefCounted
{
public:
	virtual ~MojDbStorageSeq() {}
	virtual MojErr close() = 0;
	virtual MojErr get(MojInt64& valOut) = 0;
};
