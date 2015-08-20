#include "MojDbCoreTest.h"
#include "MojTestDefs.h"
#include <stdlib.h>

void MojDbCoreTest::SetUp()
{
	MojErr err;

	const ::testing::TestInfo* const test_info =
	::testing::UnitTest::GetInstance()->current_test_info();

	path = std::string(tempFolder) + '/'
	+ test_info->test_case_name() + '-' + test_info->name();

	err = factory.init();
	MojAssertNoErr(err);

	const char* engine = getenv(EngineName);
	ASSERT_TRUE(engine) << EngineErrText;

	err = factory.createEnv(engine, env);
	MojAssertNoErr(err);

	MojObject conf;
	err = env->configure(conf);
	MojAssertNoErr(err);

	err = env->open(path.c_str());
	MojAssertNoErr(err);

	// open
	err = db.open(path.c_str(), env);
	MojAssertNoErr(err);
}

void MojDbCoreTest::TearDown()
{
	// TODO: clean DB
	MojExpectNoErr( db.close() );
}
