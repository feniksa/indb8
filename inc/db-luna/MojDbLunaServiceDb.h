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

#ifndef MOJDBLUNASERVICEDB_H
#define MOJDBLUNASERVICEDB_H

#include "core/MojReactorApp.h"
#include "core/MojGmainReactor.h"
#include "core/MojMessageDispatcher.h"
#include "db/MojDbDefs.h"
#include "db/MojDb.h"
#include "luna/MojLunaService.h"
#include "db-luna/MojDbServiceHandlerInternal.h"
#include "db-luna/MojDbServiceHandler.h"
#include "db-engine/MojDbEngineFactory.h"

class MojDbLunaServiceDb
{
public:
    MojDbLunaServiceDb(MojMessageDispatcher& dispatcher);
    MojErr init(MojReactor& reactor);
	MojErr configure(const MojObject& conf);
    MojErr open(MojGmainReactor& reactor, const MojChar* serviceName);
    MojErr openDb();
    MojErr close();

    MojDb& db() { return m_db; }
    MojLunaService& service() { return m_service; }

private:
	MojRefCountedPtr<MojDbEnv> m_env;
    MojDb m_db;
    MojLunaService m_service;
    MojRefCountedPtr<MojDbServiceHandler> m_handler;
	MojString m_databaseDir;
	MojString m_engineName;
	MojObject m_envConf;
	MojDbEngineFactory m_engineFactory;

    static MojLogger s_log;
};

#endif
