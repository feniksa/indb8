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

#ifndef MOJDBLUNASERVICEDB_H
#define MOJDBLUNASERVICEDB_H

#include "db/MojDbDefs.h"
#include "db/MojDbServiceHandlerInternal.h"
#include "db/MojDbServiceHandler.h"
#include "core/MojReactorApp.h"
#include "core/MojGmainReactor.h"
#include "core/MojMessageDispatcher.h"
#include "db/MojDb.h"
#include "luna/MojLunaService.h"

class MojDbLunaServiceDb
{
public:
    MojDbLunaServiceDb(MojMessageDispatcher& dispatcher);
    MojErr init(MojReactor& reactor);
    MojErr open(MojGmainReactor& reactor, MojDbEnv* env,
                const MojChar* serviceName, const MojChar* baseDir, const MojChar* subDir, const MojObject& conf);
    MojErr openDb(MojDbEnv* env, const MojChar* baseDir, const MojChar* subDir, const MojObject& conf);
    MojErr close();

    MojDb& db() { return m_db; }
    MojLunaService& service() { return m_service; }

private:
    MojDb m_db;
    MojLunaService m_service;
    MojRefCountedPtr<MojDbServiceHandler> m_handler;

    static MojLogger s_log;
};

#endif
