#pragma once

#include "db/MojDbDefs.h"
#include "db/MojDbStorageEngineFactory.h"
#include "core/MojAutoPtr.h"
#include <core/MojRefCount.h>
#include <vector>

class MojDbEngineFactory
{
public:
    using FactoriesContainer = std::vector<MojRefCountedPtr<MojDbStorageEngineFactory>>;

	MojDbEngineFactory();

	MojErr init();
	MojErr open();

	MojErr createEnv(const MojChar* name, MojRefCountedPtr<MojDbEnv>& envOut);
	MojErr createEngine(const MojChar* name, MojRefCountedPtr<MojDbStorageEngine>& engineOut);

    MojErr supportedEngines(const FactoriesContainer*& factoryList) const;

	MojErr addEngineFactory(MojDbStorageEngineFactory* factory);
private:
	MojErr getFactory(const MojChar* name, MojRefCountedPtr<MojDbStorageEngineFactory>& engineOut) const;

	MojRefCountedPtr<MojDbStorageEngineFactory> m_defaultFactory;
    FactoriesContainer m_factories;
	bool m_init;

	static MojLogger s_log;
};


struct MojDbEngine
{
	MojRefCountedPtr<MojDbEnv> m_env;
	MojDbEngineFactory m_factory;
};

MojErr openDB(const MojChar* path, const MojObject& engineConf, MojDb* resultDb);
