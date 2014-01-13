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


#ifndef MOJDBPERMISSIONTEST_H_
#define MOJDBPERMISSIONTEST_H_

#include "MojDbTestRunner.h"

class MojDbPermissionTest : public MojTestCase
{
public:
	MojDbPermissionTest();

	virtual MojErr run();
	virtual void cleanup();

private:
	MojErr testInvalidPermissions(MojDb& db);
	MojErr testAdminPermissions(MojDb& db);
	MojErr testKindPermissions(MojDb& db);
	MojErr testObjectPermissions(MojDb& db);
	MojErr putPermissions(MojDb& db);
	MojErr checkPermissions(MojDb& db);
	MojErr checkInvalid(const MojChar* json, MojDb& db);
    MojErr testDenyPermissions(MojDb& db);
};

#endif /* MOJDBPERMISSIONTEST_H_ */
