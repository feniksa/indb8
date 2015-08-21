#pragma once

#include "core/MojTestRunner.h"
#include "db-engine/MojDbEngineFactory.h"
#include "db/MojDbEnv.h"

class MojDbTestEnv : public MojTestCase
{
public:
	using MojTestCase::MojTestCase;

	MojErr run(const MojChar* path);

	MojRefCountedPtr<MojDbEnv>& env();
	const MojString& engineName() const { return m_engineName; }

private:
	MojString m_engineName;
	MojDbEngineFactory m_factory;
	MojRefCountedPtr<MojDbEnv> m_env;
};
