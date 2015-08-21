/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */


#include "MojDbLocaleTest.h"
#include "db/MojDb.h"
#include "MojDbTestStorageEngine.h"

static const MojChar* const MojTestKindStr =
	_T("{\"id\":\"Test:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\",\"collate\":\"secondary\"}]}]}");

static const MojChar* MojTestObjects[] = {
	_T("{\"_id\":1,\"_kind\":\"Test:1\",\"foo\":\"cote\"}"),
	_T("{\"_id\":2,\"_kind\":\"Test:1\",\"foo\":\"coté\"}"),
	_T("{\"_id\":3,\"_kind\":\"Test:1\",\"foo\":\"côte\"}"),
	_T("{\"_id\":4,\"_kind\":\"Test:1\",\"foo\":\"côté\"}"),
	NULL
};

MojDbLocaleTest::MojDbLocaleTest()
: MojDbTestEnv(_T("MojDbLocale"))
{
}

MojErr MojDbLocaleTest::run()
{
	MojErr err;

	err = MojDbTestEnv::run(MojDbTestDir);
	MojTestErrCheck(err);

	MojDb db;
	err = db.open(MojDbTestDir, env());
	MojTestErrCheck(err);

	// put kind
	MojObject obj;
	err = obj.fromJson(MojTestKindStr);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);

	err = db.updateLocale(_T("fr_CA"));
	MojTestErrCheck(err);
	err = put(db);
	MojErrCheck(err);

    err = db.updateLocale(_T("en_US"));
    MojTestErrCheck(err);
    err = checkOrder(db, _T("[1,2,3,4]"));
    MojTestErrCheck(err);

    /*
     * According to: http://userguide.icu-project.org/collation
     *   French requires that letters sorted with accents at the end of the string
     *   be sorted ahead of accents in the beginning of the string. For example,
     *   the word "côte" sorts before "coté" because the acute accent on the final
     *   "e" is more significant than the circumflex on the "o".
     *
     * Note that icu shipped with Ubuntu 12.04 and one from Yocto 1.4 doesn't
     * follow this requirements. But rather follows other what said in other
     * document: http://userguide.icu-project.org/collation/concepts#TOC-Collator-naming-scheme
     *   "Some French dictionary ordering traditions sort strings with
     *    different accents from the back of the string. This attribute is
     *    automatically set to On for the Canadian French locale (fr_CA)."
     */
    err = db.updateLocale(_T("fr_CA"));
    MojTestErrCheck(err);
    err = checkOrder(db, _T("[1,3,2,4]"));
    MojTestErrCheck(err);

	// close and reopen with test engine
	err = db.close();
	MojTestErrCheck(err);

	//setup the test storage engine
	MojRefCountedPtr<MojDbEnv> testEnv(new MojDbTestStorageEnv(env()));

	err = db.open(MojDbTestDir, testEnv);
	MojTestErrCheck(err);

	err = put(db);
	MojTestErrCheck(err);
	err = checkOrder(db, _T("[1,3,2,4]"));
	MojTestErrCheck(err);

	// fail txn commit

	MojDbTestStorageEngine* testEngine = dynamic_cast<MojDbTestStorageEngine*> (db.storageEngine());
	MojAllocCheck(testEngine);

	err = testEngine->setNextError(_T("txn.commit"), MojErrDbDeadlock);
	MojTestErrCheck(err);
	err = db.updateLocale(_T("en_US"));
	MojTestErrExpected(err, MojErrDbDeadlock);
	// make sure we still have fr ordering
	err = checkOrder(db, _T("[1,3,2,4]"));
	MojTestErrCheck(err);
	err = put(db);
	MojTestErrCheck(err);
	err = checkOrder(db, _T("[1,3,2,4]"));
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbLocaleTest::put(MojDb& db)
{
	// make sure that we are still consistent by deleting and re-adding
	MojDbQuery query;
	MojErr err = query.from(_T("Test:1"));
	MojTestErrCheck(err);
	MojUInt32 count = 0;
	err = db.del(query, count);
	MojTestErrCheck(err);

	for (const MojChar** i = MojTestObjects; *i != NULL; ++i) {
		MojObject obj;
		err = obj.fromJson(*i);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbLocaleTest::checkOrder(MojDb& db, const MojChar* expectedJson)
{
	MojObject expected;
	MojErr err = expected.fromJson(expectedJson);
	MojTestErrCheck(err);

    MojTestAssert (expected.type() == MojObject::TypeArray);

	MojObject::ConstArrayIterator i = expected.arrayBegin();

	MojDbQuery query;
	err = query.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query.order(_T("foo"));
	MojTestErrCheck(err);
	MojDbCursor cur;
	err = db.find(query, cur);
	MojTestErrCheck(err);
	for (;;) {
		MojObject obj;
		bool found = false;
		err = cur.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		MojTestAssert(i != expected.arrayEnd());
		MojObject id;
		MojTestAssert(obj.get(MojDb::IdKey, id));
		MojInt64 idInt = id.intValue();
		MojUnused(idInt);

		MojTestAssert(id == *i++);
	}
	return MojErrNone;
}

void MojDbLocaleTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
