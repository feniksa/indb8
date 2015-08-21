#include "db-engine/MojDbEngineFactory.h"

#include "core/MojUtil.h"
#include "core/MojString.h"

#include <gtest/gtest.h>


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
}
