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

	const char* const MojKindStr3 =
		_T("{\"id\":\"TestAppA:1\",")
		_T("\"owner\":\"TestAppA\",")
		_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]}]}");

	const char* const MojKindStr4 =
		_T("{\"id\":\"TestAppB:1\",")
		_T("\"owner\":\"TestAppB\",")
		_T("\"indexes\":[{\"name\":\"bar\",\"props\":[{\"name\":\"bar\"}]}]}");

	const std::set<std::string> KindIds {_T("Test:1"), _T("MyTest:1")};
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
		err = i->getRequired(_T("id"), id);
		MojAssertNoErr(err);

		auto etalon_iter = KindIds.find(id.data());
		EXPECT_NE(KindIds.end(), etalon_iter) << "DB8 return kind, that wasn't registered inside database";
	}
}

TEST_F(KindsTest, get_kinds_with_permission_check)
{
	MojErr err;

	MojObject kindObj3;
	MojObject kindObj4;

	err = kindObj3.fromJson(MojKindStr3);
	MojAssertNoErr(err);

	err = kindObj4.fromJson(MojKindStr4);
	MojAssertNoErr(err);

	MojDbReq req3(false);
	err = req3.domain(_T("TestAppA"));
	MojAssertNoErr(err);

	err = db.putKind(kindObj3, MojDb::FlagNone, req3);
	MojAssertNoErr(err);

	MojDbReq req4(false);
	err = req4.domain(_T("TestAppB"));
	MojAssertNoErr(err);
	err = db.putKind(kindObj4, MojDb::FlagNone, req4);
	MojAssertNoErr(err);

	MojDbReq req5(false);
	err = req5.domain(_T("TestAppA"));
	MojAssertNoErr(err);

	MojVector<MojObject> list;
	err = db.getKindList(list, req5);
	MojAssertNoErr(err);

	EXPECT_EQ(1, list.size());

	for (MojVector<MojObject>::ConstIterator i = list.begin(); i != list.end(); ++i) {
		MojString id;
		err = i->getRequired(_T("id"), id);
		MojAssertNoErr(err);

		EXPECT_STREQ(_T("TestAppA:1"), id.data());
	}

	// get from non exist app
	MojDbReq req6(false);
	err = req6.domain(_T("TestAppNonExists"));
	MojAssertNoErr(err);

	list.clear();
	err = db.getKindList(list, req6);
	MojAssertNoErr(err);

	EXPECT_EQ(0, list.size());
}
