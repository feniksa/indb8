/* @@@LICENSE
*
*      Copyright (c) 2009-2015 LG Electronics, Inc.
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


#include "db/MojDbExtractor.h"
#include "db/MojDbTextTokenizer.h"
#include "db/MojDbUtils.h"

const MojChar* const MojDbExtractor::NameKey = _T("name");
const MojChar* const MojDbExtractor::CollateKey = _T("collate");


MojErr MojDbExtractor::fromObject(const MojObject& obj, const MojChar* locale)
{
	// name
	MojErr err = obj.getRequired(NameKey, m_name);
	MojErrCheck(err);

	// collation
	bool found = false;
	MojString collate;
	err = obj.get(CollateKey, collate, found);
	MojErrCheck(err);
	if (found) {
		MojDbCollationStrength strength;
		err = MojDbUtils::collationFromString(collate, strength);
		MojErrCheck(err);
		if (strength != MojDbCollationInvalid) {
			m_collation = strength;
		}
	}
	return MojErrNone;
}
