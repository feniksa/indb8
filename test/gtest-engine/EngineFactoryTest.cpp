#include "db-engine/MojDbEngineEnvSingle.h"
#include "db-engine/MojDbEngineFactory.h"

#include <gtest/gtest.h>
#include "core/MojUtil.h"


struct EngineFactorySuite : public ::testing::Test
{
	MojDbEngineFactory factory;
	MojString TempFolder;

	void SetUp()
	{
		MojErr err = factory.init();
		ASSERT_EQ(MojErrNone, err);

		char buf[128] = "/tmp/mojodbengine-test-dir-XXXXXX";
		const char* res = mkdtemp(buf);

		if (res) {
			err = TempFolder.assign(buf, sizeof(buf)); // fallback
			ASSERT_EQ(MojErrNone, err);
		} else {
			err = TempFolder.assign(_T("/tmp/mojodbengine-test-dir")); // fallback
			ASSERT_EQ(MojErrNone, err);
		}
	}

	void TearDown()
	{
		// cleanup temp folder
		MojRmDirRecursive(TempFolder.data());
	}
};

TEST_F(EngineFactorySuite, init)
{
	MojErr err;
	MojDbEngineEnvSingle env(factory);

	err = env.init(_T("sandwich"));
	ASSERT_EQ(MojErrNone, err);

	MojObject conf;
	err = env.configure(conf);
	ASSERT_EQ(MojErrNone, err);

	err = env.openEnv(TempFolder);
	ASSERT_EQ(MojErrNone, err);

	err = env.openStorage(TempFolder);
	ASSERT_EQ(MojErrNone, err);

	err = env.close();
	ASSERT_EQ(MojErrNone, err);

	// and try to reclose
	err = env.close();
	ASSERT_EQ(MojErrNone, err);
}
