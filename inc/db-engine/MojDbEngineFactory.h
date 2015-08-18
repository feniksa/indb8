#pragma once

#include "db/MojDbDefs.h"
#include "db/MojDbStorageEngineFactory.h"
#include "core/MojAutoPtr.h"
#include <core/MojRefCount.h>
#include <vector>

class MojDbEngineFactory
{
	using FactoryType = std::vector<MojRefCountedPtr<MojDbStorageEngineFactory>>;

public:
	MojDbEngineFactory();

	MojErr init();
	MojErr open();

	MojErr createEnv(const MojChar* name, MojRefCountedPtr<MojDbEnv>& engineOut);
	MojErr createEngine(const MojChar* name, MojRefCountedPtr<MojDbStorageEngine>& engineOut);

	MojErr supportedEngines(const FactoryType*& factoryList) const;

	MojErr addEngineFactory(MojDbStorageEngineFactory* factory);
private:
	MojErr getFactory(const MojChar* name, MojRefCountedPtr<MojDbStorageEngineFactory>& engineOut);

	MojRefCountedPtr<MojDbStorageEngineFactory> m_defaultFactory;
	FactoryType m_factory;
	bool m_init;

	static MojLogger s_log;
};


struct MojDbEngine
{
	MojRefCountedPtr<MojDbEnv> m_env;
	MojDbEngineFactory m_factory;
};

MojErr openDB(const MojChar* path, const MojObject& engineConf, MojDb* resultDb);
