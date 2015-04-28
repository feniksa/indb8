#pragma once

#include "db/MojDbDefs.h"
#include "core/MojRefCount.h"

class MojDbStorageEngineFactory : public MojRefCounted
{
public:
	virtual ~MojDbStorageEngineFactory() {}
	virtual MojErr create(MojRefCountedPtr<MojDbStorageEngine>& engineOut) const = 0;
	virtual MojErr createEnv(MojRefCountedPtr<MojDbEnv>& envOut) const = 0;
	virtual const MojChar* name() const = 0;
};
