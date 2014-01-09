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


#ifndef MOJDBBERKELEYFACTORY_H_
#define MOJDBBERKELEYFACTORY_H_

#include "db/MojDbDefs.h"
#include "db/MojDbStorageEngine.h"

class MojDbBerkeleyFactory : public MojDbStorageEngineFactory
{
public:

	virtual MojErr create(MojRefCountedPtr<MojDbStorageEngine>& engineOut) const;
	virtual MojErr createEnv(MojRefCountedPtr<MojDbEnv>& envOut) const;
	virtual const MojChar* name() const;
};

#endif /* MOJDBBERKELEYFACTORY_H_ */
