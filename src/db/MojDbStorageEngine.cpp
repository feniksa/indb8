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


#include "db/MojDbStorageEngine.h"
#include "core/MojObjectBuilder.h"
#include "core/MojJson.h"

MojRefCountedPtr<MojDbStorageEngineFactory> MojDbStorageEngine::m_factory;

MojErr MojDbStorageEngine::createDefaultEngine(MojRefCountedPtr<MojDbStorageEngine>& engineOut)
{
   if(m_factory.get() == 0)
      MojErrThrowMsg(MojErrDbStorageEngineNotFound, _T("Storage engine is not set"));
   MojErr err = m_factory->create(engineOut);
   MojErrCheck(err);
   return MojErrNone;
}

MojErr MojDbStorageEngine::createEngine(const MojChar* name, MojRefCountedPtr<MojDbStorageEngine>& engineOut)
{
	MojAssert(name);

	if (MojStrCmp(name, m_factory->name()) == 0) {
		MojErr err = m_factory->create(engineOut);
		MojErrCheck(err);
		return MojErrNone;
	}
	MojErrThrowMsg(MojErrDbStorageEngineNotFound, _T("Storage engine not found: '%s'"), name);
}
MojErr MojDbStorageEngine::setEngineFactory(MojDbStorageEngineFactory* factory)
{
   m_factory.reset(factory);
   return MojErrNone;
}


MojDbStorageEngine::MojDbStorageEngine()
{
}
