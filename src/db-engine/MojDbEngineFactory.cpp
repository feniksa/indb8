#include "db-engine/MojDbEngineFactory.h"
#include "db/MojDbStorageEngineFactory.h"

#ifdef MOJ_USE_SANDWICH
#include "db-engine/sandwich/MojDbSandwichFactory.h"
#endif

#ifdef MOJ_USE_LDB
#include "db-engine/leveldb/MojDbLevelFactory.h"
#endif

#ifdef MOJ_USE_BDB
#include "db-engine/berkeley/MojDbBerkeleyFactory.h"
#endif

MojRefCountedPtr<MojDbStorageEngineFactory> MojDbEngineFactory::m_factory;

MojDbEngineFactory::factory_container_t MojDbEngineFactory::m_factories {
#ifdef MOJ_USE_SANDWICH
	new MojDbSandwichFactory(),
#endif
#ifdef MOJ_USE_LDB
	new MojDbLevelFactory(),
#endif
#ifdef MOJ_USE_BDB
	new MojDbBerkeleyFactory()
#endif
};

MojErr MojDbEngineFactory::createDefaultEngine(MojRefCountedPtr<MojDbStorageEngine>& engineOut)
{
	if(m_factory.get() == 0)
		MojErrThrowMsg(MojErrDbStorageEngineNotFound, _T("Storage engine is not set"));

	MojErr err = m_factory->create(engineOut);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojDbEngineFactory::createEngine(const MojChar* name, MojRefCountedPtr<MojDbStorageEngine>& engineOut)
{
	MojAssert(name);

	MojErr err;
	MojRefCountedPtr<MojDbStorageEngineFactory> factory;

	err = engineFactory(name, factory);
	MojErrCheck(err);

	err = factory->create(engineOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbEngineFactory::setEngineFactory(MojDbStorageEngineFactory* factory)
{
	m_factory.reset(factory);
	return MojErrNone;
}

MojErr MojDbEngineFactory::engineFactory(MojRefCountedPtr<MojDbStorageEngineFactory>& outFactory)
{
	if (m_factory.get()) {
		outFactory = m_factory;
		return MojErrNone;
	}
	MojErrThrowMsg(MojErrDbStorageEngineNotFound, _T("Storage engine not set"));
}

MojErr MojDbEngineFactory::engineFactory(const MojChar* name, MojRefCountedPtr<MojDbStorageEngineFactory>& outFactory)
{
	if (m_factory.get()) {
		if (MojStrCmp(name, m_factory->name()) == 0) {
			outFactory = m_factory;
			return MojErrNone;
		}
	}

	for (const auto factory : m_factories) {
		if (MojStrCmp(name, factory->name()) == 0) {
			outFactory = factory;
			return MojErrNone;
		}
	}

	MojErrThrowMsg(MojErrDbStorageEngineNotFound, _T("Storage engine not found: '%s'"), name);
}
