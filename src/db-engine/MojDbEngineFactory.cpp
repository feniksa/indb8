#include "db-engine/MojDbEngineFactory.h"

#ifdef MOJ_USE_BDB
	#include "db-engine/berkeley/MojDbBerkeleyFactory.h"
#endif

#ifdef MOJ_USE_LDB
	#include "db-engine/leveldb/MojDbLevelFactory.h"
#endif

#ifdef MOJ_USE_SANDWICH
	#include "db-engine/sandwich/MojDbSandwichFactory.h"
#endif

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
#ifdef MOJ_USE_BDB
	err = addEngineFactory(new MojDbBerkeleyFactory());
	MojErrCheck(err);
#endif

#ifdef MOJ_USE_LDB
	err = addEngineFactory(new MojDbLevelFactory());
	MojErrCheck(err);
#endif

#ifdef MOJ_USE_SANDWICH
	err = addEngineFactory(new MojDbSandwichFactory());
	MojErrCheck(err);
#endif

	return MojErrNone;
}

MojErr MojDbEngineFactory::getFactory(const MojChar* name, MojRefCountedPtr<MojDbStorageEngineFactory>& engineOut)
{
	for (FactoryType::iterator i = m_factory.begin(); i != m_factory.end(); ++i) {
		if (MojStrCmp(i->get()->name(), name) == 0)
		{
			MojLogDebug(s_log, "Found engine in factory: %s", name);

			engineOut = *i;
			return MojErrNone;
		}
	}

	MojErrThrowMsg(MojErrDbStorageEngineNotFound, _T("Storage Factory not found: '%s'"), name);
}

MojErr MojDbEngineFactory::createEngine(const MojChar* name, MojRefCountedPtr<MojDbStorageEngine>& engineOut)
{
	MojLogTrace(s_log);

	MojAssert(name);

	MojErr err;
	MojRefCountedPtr<MojDbStorageEngineFactory> factory;

	err = getFactory(name, factory);
	MojErrCheck(err);

	err = factory->create(engineOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbEngineFactory::supportedEngines(const FactoryType*& factoryList) const
{
	MojLogTrace(s_log);

	factoryList = &m_factory;
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

#include <algorithm>

MojErr MojDbEngineFactory::addEngineFactory(MojDbStorageEngineFactory* factory)
{
	MojLogTrace(s_log);
	MojAssert(factory);

	std::find_if(m_factory.begin(),m_factory.end(), [](FactoryType::value_type) { return true; });

	for (FactoryType::iterator i = m_factory.begin(); i != m_factory.end(); ++i) {
		if (MojStrCmp(i->get()->name(), factory->name()) == 0) {
			if (i->get() != factory)
				*i = factory;

			return MojErrNone;
		}
	}

	m_factory.push_back(factory);

	return MojErrNone;
}

/*
MojErr openDB(const MojChar* path, const MojChar* engineName, const MojObject* engineConf, MojDb* resultDb)
{

	MojRefCountedPtr<MojDbStorageEngine> engineOut;

	if (engineName) {
		err = MojDbEngineFactory::createEngine(engineName, engineOut);
		MojErrCheck(err);
	} else {
		err = MojDbEngineFactory::createDefaultEngine(engineOut);
		MojErrCheck(err);
	}

	MojAssert(engineOut.get());

	if (engineConf) {
		err = engineOut->configure(*engineConf);
		MojErrCheck(err);
	}

	err = engineOut->open(path);
	MojErrCheck(err);

	return MojErrNone;
}*/
