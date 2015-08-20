#include "MojDbTestEnv.h"

MojErr MojDbTestEnv::run()
{
	MojErr err;

	err = m_factory.init();
	MojErrCheck(err);

	err = m_factory.createEnv("sandwich", m_env);
	MojErrCheck(err);

	return MojErrNone;
}
