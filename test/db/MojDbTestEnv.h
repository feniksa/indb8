#pragma once

#include "core/MojTestRunner.h"
#include "db-engine/MojDbEngineFactory.h"
#include "db/MojDbEnv.h"

class MojDbTestEnv : public MojTestCase
{
public:
	using MojTestCase::MojTestCase;

	MojErr run();

	MojRefCountedPtr<MojDbEnv>& env();

private:
	MojDbEngineFactory m_factory;
	MojRefCountedPtr<MojDbEnv> m_env;
};
