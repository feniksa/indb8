#pragma once

#include "core/MojRefCount.h"

class MojDbEnv : public MojRefCounted
{
public:
	virtual ~MojDbEnv() {}
	virtual MojErr configure(const MojObject& conf) = 0;
	virtual MojErr open(const MojChar* path) = 0;
	virtual MojErr close() = 0;
};
