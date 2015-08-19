#pragma once

#include "core/MojTestRunner.h"
#include "db-engine/MojDbEngineFactory.h"
#include "db/MojDbEnv.h"

class MojDbTestEnv : public MojTestCase
{
public:
	using MojTestCase::MojTestCase;

	MojErr run();

	MojDbEnv* env() { MojAssert(m_env.get()); return m_env.get(); }

private:
	MojDbEngineFactory m_factory;
	MojRefCountedPtr<MojDbEnv> m_env;
};
