#pragma once

#include "db/MojDbDefs.h"
#include "core/MojRefCount.h"
#include <memory>
#include <vector>

class MojDbEngineFactory : public MojRefCounted
{
public:
	static MojErr createDefaultEngine(MojRefCountedPtr<MojDbStorageEngine>& engineOut);
	static MojErr createEngine(const MojChar* name, MojRefCountedPtr<MojDbStorageEngine>& engineOut);
	static MojErr createEnv(MojRefCountedPtr<MojDbEnv>& envOut);
	static MojErr setEngineFactory(MojDbStorageEngineFactory* factory);
	static MojErr engineFactory(MojRefCountedPtr<MojDbStorageEngineFactory>& outFactory);
	static MojErr engineFactory(const MojChar* name, MojRefCountedPtr<MojDbStorageEngineFactory>& outFactory);

	using factory_ref_t = MojRefCountedPtr<MojDbStorageEngineFactory>;
	using factory_container_t = std::vector<factory_ref_t>;

private:
	static factory_container_t m_factories;
	static factory_ref_t m_factory;
};
