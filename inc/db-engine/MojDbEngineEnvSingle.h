#pragma once

#include "db/MojDb.h"
#include "db/MojDbEnv.h"
#include "db-engine/MojDbEngineFactory.h"

class MojDbEngineEnvSingle : public MojNoCopy
{
public:
	MojDbEngineEnvSingle(MojDbEngineFactory& factory);

	~MojDbEngineEnvSingle();

	MojErr init(const MojChar* engineName);
	MojErr openEnv(const MojChar* envPath);
	MojErr openStorage(const MojChar* storagePath);
	MojErr configure(const MojObject& conf);
	MojErr close();

	inline MojRefCountedPtr<MojDbStorageEngine> engine() const { return m_engine; }
	inline MojRefCountedPtr<MojDbEnv> env() const { return m_env; }

private:
	MojRefCountedPtr<MojDbStorageEngine> m_engine;
	MojRefCountedPtr<MojDbEnv> m_env;

	MojString m_engineName;

	MojDbEngineFactory& m_factory;
	bool m_envIsOpen;

	static MojLogger s_log;
};
