#include "MojDbCoreTest.h"

void MojDbCoreTest::SetUp()
{
	MojErr err;

	const ::testing::TestInfo* const test_info =
	::testing::UnitTest::GetInstance()->current_test_info();

	path = std::string(tempFolder) + '/'
	+ test_info->test_case_name() + '-' + test_info->name();

	err = factory.init();
	MojAssertNoErr(err);

	err = factory.createEnv("sandwich", env);
	MojAssertNoErr(err);

	// open
	err = db.open(path.c_str(), env.get());
	MojAssertNoErr(err);
}

void MojDbCoreTest::TearDown()
{
	// TODO: clean DB
	MojExpectNoErr( db.close() );
}
