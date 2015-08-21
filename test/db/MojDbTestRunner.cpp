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

#include <string>
#include <unistd.h>
#include <time.h>

#include "MojDbTestRunner.h"
#include "MojDbWhereTest.h"
#include "MojDbBulkTest.h"
#include "MojDbConcurrencyTest.h"
#include "MojDbCrudTest.h"
#include "MojDbDumpLoadTest.h"
#include "MojDbIndexTest.h"
#include "MojDbKindTest.h"
#include "MojDbLocaleTest.h"
#include "MojDbPermissionTest.h"
#include "MojDbPurgeTest.h"
#include "MojDbQueryTest.h"
#include "MojDbQueryFilterTest.h"
#include "MojDbQuotaTest.h"
#include "MojDbRevTest.h"
#include "MojDbRevisionSetTest.h"
#include "MojDbSearchTest.h"
#include "MojDbDistinctTest.h"
#include "MojDbTextCollatorTest.h"
#include "MojDbTextTokenizerTest.h"
#include "MojDbWatchTest.h"
#include "MojDbTxnTest.h"
#include "MojDbCursorTxnTest.h"
#include <sstream>

std::string getTestDir()
{
	struct timespec spec;

	if (clock_gettime(CLOCK_REALTIME, &spec) != 0)
		throw std::exception();

	std::stringstream stream;
	stream << "/tmp/mojodb-test-dir-" << spec.tv_sec << "-" << spec.tv_nsec;

	return stream.str();
}

const std::string mojDbTestDirString = getTestDir();
const MojChar* const MojDbTestDir = mojDbTestDirString.c_str();

int main(int argc, char** argv)
{
	MojDbTestRunner runner;

	return runner.main(argc, argv);
}

void MojDbTestRunner::runTests()
{
	test(MojDbBulkTest());
	test(MojDbConcurrencyTest());
	test(MojDbCrudTest());
	test(MojDbDumpLoadTest());
	test(MojDbIndexTest());
	test(MojDbKindTest());
	test(MojDbLocaleTest());
	test(MojDbPermissionTest());
	test(MojDbPurgeTest());
	test(MojDbQueryTest());
	test(MojDbQueryFilterTest());
	test(MojDbQuotaTest());
	test(MojDbRevTest());
	test(MojDbRevisionSetTest());
	test(MojDbSearchTest());
	test(MojDbDistinctTest());
	test(MojDbTextCollatorTest());
	test(MojDbTextTokenizerTest());
	test(MojDbWatchTest());
	test(MojDbTxnTest());
	test(MojDbWhereTest());
	test(MojDbCursorTxnTest());
}
