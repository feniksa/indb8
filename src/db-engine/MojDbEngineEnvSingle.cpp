#include "db-engine/MojDbEngineEnvSingle.h"

MojLogger MojDbEngineEnvSingle::s_log("db-engine.env-single");

MojDbEngineEnvSingle::MojDbEngineEnvSingle(MojDbEngineFactory& factory)
: m_factory(factory),
  m_envIsOpen(false)
{
}

MojDbEngineEnvSingle::~MojDbEngineEnvSingle()
{
	close();
}

MojErr MojDbEngineEnvSingle::init(const MojChar* engineName)
{
	MojAssert(!m_engine.get());
	MojAssert(!m_env.get());
	MojAssert(engineName);

	MojErr err;

	err = m_factory.createEnv(engineName, m_env);
	MojErrCheck(err);

	err = m_factory.createEngine(engineName, m_engine);
	MojErrCheck(err);

	err = m_engineName.assign(engineName);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbEngineEnvSingle::openEnv(const MojChar* envPath)
{
	MojAssert(m_engine.get());
	MojAssert(m_env.get());
	MojAssert(envPath);

	MojErr err;

	err = m_env->open(envPath);
	MojErrCheck(err);

	m_envIsOpen = true;

	return MojErrNone;
}

MojErr MojDbEngineEnvSingle::openStorage(const MojChar* storagePath)
{
	MojAssert(m_engine.get());
	MojAssert(m_env.get());
	MojAssert(storagePath);

	if (!m_envIsOpen)
		MojErrThrowMsg(MojErrNotOpen, "Environment not opened");

	MojErr err;

	err = m_engine->open(storagePath, m_env.get());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbEngineEnvSingle::configure(const MojObject& conf)
{
	MojAssert(m_engine.get());
	MojAssert(m_env.get());

	MojErr err;

	MojObject envConf;
	if (!conf.get(m_engineName, envConf)) {
		MojLogWarning(s_log, "No configuration provided for \"%s\" engine", m_engineName.data());
	}

	err = m_env->configure(envConf);
	MojErrCheck(err);

	// as we have ability to open only one engine at one place, we path env and engine conf same values
	err = m_engine->configure(envConf);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbEngineEnvSingle::close()
{
	MojErr err = MojErrNone;
	MojErr errClose;

	if (m_engine.get() && m_env.get()) {
		errClose = m_engine->close();
		MojErrAccumulate(err, errClose);

		errClose = m_env->close();
		MojErrAccumulate(err, errClose);

		m_envIsOpen = false;
	}

	MojErrCheck(err);

	return MojErrNone;
}
