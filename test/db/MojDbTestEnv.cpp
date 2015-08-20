#include "MojDbTestEnv.h"

#include "MojTestDefs.h"
#include <stdlib.h>

MojErr MojDbTestEnv::run(const MojChar* path)
{
	MojErr err;

	err = m_factory.init();
	MojErrCheck(err);

	const char* engine = getenv(EngineName);
	if (!engine) {
		MojErrThrowMsg(MojErrDbStorageEngineNotFound, EngineErrText);
	}

	err = m_factory.createEnv(engine, m_env);
	MojErrCheck(err);

	MojObject conf;

	err = m_env->configure(conf);
	MojErrCheck(err);

	err = m_env->open(path);
	MojErrCheck(err);

	err = m_engineName.assign(engine);
	MojErrCheck(err);

	return MojErrNone;
}

MojRefCountedPtr<MojDbEnv>& MojDbTestEnv::env()
{
	MojAssert(m_env.get());
	return m_env;
}
