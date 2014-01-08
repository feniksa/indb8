/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2013 LG Electronics, Inc.
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
 * LICENSE@@@
 ****************************************************************/

#include "db-luna/MojDbLunaServiceDb.h"
#include "db/MojDbServiceDefs.h"
#include "core/MojApp.h"

MojLogger MojDbLunaServiceDb::s_log(_T("db-luna.dbservice"));

MojDbLunaServiceDb::MojDbLunaServiceDb(MojMessageDispatcher& dispatcher)
: m_service(true, &dispatcher)
{
}

MojErr MojDbLunaServiceDb::init(MojReactor& reactor)
{
    m_handler.reset(new MojDbServiceHandler(m_db, reactor));
    MojAllocCheck(m_handler.get());

    return MojErrNone;
}

MojErr MojDbLunaServiceDb::open(MojGmainReactor& reactor, MojDbEnv* env,
                                const MojChar* serviceName, const MojChar* baseDir, const MojChar* subDir, const MojObject& conf)
{
    MojAssert(serviceName && baseDir && subDir);
    MojLogTrace(s_log);

    // open service
    MojErr err = m_service.open(serviceName);
    MojErrCheck(err);
    err = m_service.attach(reactor.impl());
    MojErrCheck(err);
    // open handler
    err = m_handler->open();
    MojErrCheck(err);
    err = m_service.addCategory(MojDbServiceDefs::Category, m_handler.get());
    MojErrCheck(err);
    // open db
    err = openDb(env, baseDir, subDir, conf);
    if (err != MojErrNone) {
        MojString msg;
        MojErrToString(err, msg);
        MojLogError(s_log, _T("Error opening %s/%s - %s (%d)"), baseDir, subDir, msg.data(), (int) err);
    }
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServiceDb::openDb(MojDbEnv* env, const MojChar* baseDir, const MojChar* subDir, const MojObject& conf)
{
    MojAssert(env && baseDir && subDir);

    // create engine with shared env
    MojString path;
    MojErr err = path.format(_T("%s/%s"), baseDir, subDir);
    MojErrCheck(err);
    //  MojRefCountedPtr<MojDbBerkeleyEngine> engine(new MojDbBerkeleyEngine);
    MojRefCountedPtr<MojDbStorageEngine> engine;
    MojDbStorageEngine::engineFactory()->create(engine);
    MojAllocCheck(engine.get());
    err = engine->configure(conf);
    MojErrCheck(err);
    err = engine->open(path, env);
    MojErrCheck(err);

    // open db
    err = m_db.open(path, engine.get());
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServiceDb::close()
{
    MojErr err = MojErrNone;

    m_handler.reset();
    MojErr errClose = m_service.close();
    MojErrAccumulate(err, errClose);
    errClose = m_db.close();
    MojErrAccumulate(err, errClose);

    return err;
}
