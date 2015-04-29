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


#pragma once

#include "db/MojDbDefs.h"
#include "db/MojDbKey.h"
#include "db/MojDbTextCollator.h"
#include "db/MojDbTextTokenizer.h"
#include "core/MojObject.h"
#include "core/MojSet.h"

class MojDbExtractor : public MojRefCounted
{
public:
	static const MojChar* const CollateKey;
	static const MojChar* const NameKey;

	using KeySet = MojSet<MojDbKey>;

	MojDbExtractor() : m_collation(MojDbCollationInvalid) {}
	virtual ~MojDbExtractor() {}
	virtual MojErr fromObject(const MojObject& obj, const MojChar* locale) = 0;
	virtual MojErr updateLocale(const MojChar* locale) = 0;
	virtual MojErr vals(const MojObject& obj, KeySet& valsOut) const = 0;

	const MojString& name() const { return m_name; }
	MojDbCollationStrength collation() const { return m_collation; }

protected:
	MojString m_name;
	MojDbCollationStrength m_collation;
};
