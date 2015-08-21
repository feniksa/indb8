#include "db-engine/MojDbEngineFactory.h"

#ifdef BUILD_ENGINE_BERKELEY
	#include "db-engine/berkeley/MojDbBerkeleyFactory.h"
#endif

#ifdef BUILD_ENGINE_LEVELDB
	#include "db-engine/leveldb/MojDbLevelFactory.h"
#endif

#ifdef BUILD_ENGINE_SANDWICH
	#include "db-engine/sandwich/MojDbSandwichFactory.h"
#endif

#include <algorithm>

MojLogger MojDbEngineFactory::s_log(_T("db-factory.factory"));

MojDbEngineFactory::MojDbEngineFactory()
: m_init(false)
{
}

MojErr MojDbEngineFactory::init()
{
	MojLogTrace(s_log);

	MojAssert(!m_init);

	m_init = true;

	MojErr err;
#ifdef BUILD_ENGINE_BERKELEY
	#define HAVE_ENGINE

	MojRefCountedPtr<MojDbStorageEngineFactory> berkeleyFactory(new MojDbBerkeleyFactory());
	err = addEngineFactory(berkeleyFactory);
	MojErrCheck(err);
#endif

#ifdef BUILD_ENGINE_LEVELDB
	#define HAVE_ENGINE

	MojRefCountedPtr<MojDbStorageEngineFactory> leveldbFactory(new MojDbLevelFactory());
	err = addEngineFactory(leveldbFactory);
	MojErrCheck(err);
#endif

#ifdef BUILD_ENGINE_SANDWICH
	#define HAVE_ENGINE

	MojRefCountedPtr<MojDbStorageEngineFactory> sandwichFactory(new MojDbSandwichFactory());
	err = addEngineFactory(sandwichFactory);
	MojErrCheck(err);
#endif

#ifndef HAVE_ENGINE
#error "MojDbEngineFactory compiled without any engine support. Please, specify what ENGINE with should use"
#endif

	return MojErrNone;
}

MojErr MojDbEngineFactory::getFactory(const MojChar* name, MojRefCountedPtr<MojDbStorageEngineFactory>& engineOut) const
{
    MojLogTrace(s_log);
    if (!name) {
		MojErrThrowMsg(MojErrDbStorageEngineNotFound, _T("Engine name is null"));
	}

    auto iter = std::find_if(m_factories.begin(), m_factories.end(), [name](const FactoriesContainer::value_type& iter) {
        return (MojStrCmp(iter->name(), name) == 0);
    });

    if (iter != m_factories.end()) {
        MojLogDebug(s_log, "Found engine in factory: %s", name);

        engineOut = *iter;
        return MojErrNone;
    }

	MojErrThrowMsg(MojErrDbStorageEngineNotFound, _T("Storage Factory not found: '%s'"), name);
}

MojErr MojDbEngineFactory::supportedEngines(const FactoriesContainer*& factoryList) const
{
	MojLogTrace(s_log);

	factoryList = &m_factories;
	return MojErrNone;
}

MojErr MojDbEngineFactory::createEnv(const MojChar* name, MojRefCountedPtr<MojDbEnv>& envOut)
{
	MojLogTrace(s_log);

	MojErr err;
	MojRefCountedPtr<MojDbStorageEngineFactory> factory;

	err = getFactory(name, factory);
	MojErrCheck(err);

	err = factory->createEnv(envOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbEngineFactory::addEngineFactory(MojRefCountedPtr<MojDbStorageEngineFactory>& factory)
{
	MojLogTrace(s_log);
	MojAssert(factory.get());

    auto iter = std::find_if(m_factories.begin(), m_factories.end(), [factory](const FactoriesContainer::value_type& iter) {
        return (MojStrCmp(iter->name(), factory->name()) == 0);
    });

    if (iter != m_factories.end()) {
		if (*iter != factory) {
			*iter = factory;
		}
    } else {
        m_factories.push_back(factory);
    }

	return MojErrNone;
}
