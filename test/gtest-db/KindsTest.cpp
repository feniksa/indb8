#include "db/MojDb.h"
#include "MojDbCoreTest.h"
#include "core/MojObjectBuilder.h"

namespace
{
	const char* const MojKindStr1 =
		_T("{\"id\":\"Test:1\",")
			_T("\"owner\":\"mojodb.admin\",")
		_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\"}]}]}");

	const char* const MojKindStr2 =
		_T("{\"id\":\"MyTest:1\",")
		_T("\"owner\":\"mojodb.admin\",")
		_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]}]}");

	const std::set<std::string> KindIds {_T("_kinds/Test:1"), _T("_kinds/MyTest:1")};
}

struct KindsTest : public MojDbCoreTest
{

	void SetUp()
	{
		MojDbCoreTest::SetUp();
	}
};

TEST_F(KindsTest, get_kinds)
{
	MojErr err;

	MojObject kindObj1;
	MojObject kindObj2;

	err = kindObj1.fromJson(MojKindStr1);
	MojAssertNoErr(err);

	err = kindObj2.fromJson(MojKindStr2);
	MojAssertNoErr(err);

	MojDbReq req;

	err = db.putKind(kindObj1, MojDb::FlagNone, req);
	MojAssertNoErr(err);

	err = db.putKind(kindObj2, MojDb::FlagNone, req);
	MojAssertNoErr(err);

	MojVector<MojObject> list;
	err = db.getKindList(list, req);
	MojAssertNoErr(err);

	EXPECT_EQ(KindIds.size(), list.size());

	for (MojVector<MojObject>::ConstIterator i = list.begin(); i != list.end(); ++i) {
		MojString id;
		err = i->getRequired(_T("_id"), id);
		MojAssertNoErr(err);

		auto etalon_iter = KindIds.find(id.data());
		EXPECT_NE(KindIds.end(), etalon_iter) << "DB8 return kind, that wasn't registered inside database";
	}
}
