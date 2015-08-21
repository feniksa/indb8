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
#include "db/MojDbEnv.h"

MojLogger MojDbLunaServiceDb::s_log(_T("db-luna.dbservice"));

MojDbLunaServiceDb::MojDbLunaServiceDb(MojMessageDispatcher& dispatcher)
: m_service(true, &dispatcher)
{
}

MojErr MojDbLunaServiceDb::configure(const MojObject& conf)
{
	MojErr err;

	err = conf.getRequired(_T("engine"), m_engineName);
	MojErrCheck(err);

	err = conf.getRequired(m_engineName, m_envConf);
	MojErrCheck(err);

	MojObject dbConf;
	err = conf.getRequired(_T("db"), dbConf);
	MojErrCheck(err);

	err = dbConf.getRequired(_T("path"), m_databaseDir);
	MojErrCheck(err);

	err = m_db.configure(dbConf);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbLunaServiceDb::init(MojReactor& reactor)
{
	MojErr err;

    m_handler.reset(new MojDbServiceHandler(m_db, reactor));
    MojAllocCheck(m_handler.get());

	err = m_engineFactory.init();
	MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServiceDb::open(MojGmainReactor& reactor, const MojChar* serviceName)
{
    MojAssert(serviceName);
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
    err = openDb();
    if (err != MojErrNone) {
        MojString msg;
        MojErrToString(err, msg);
        MojLogError(s_log, _T("Error opening %s - %s (%d)"), m_databaseDir.data(), msg.data(), (int) err);
    }
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServiceDb::openDb()
{
	MojAssert(!m_env.get());
	MojAssert(!m_databaseDir.empty());

    MojErr err;

	err = m_engineFactory.createEnv(m_engineName, m_env);
	MojErrCheck(err);
	MojAllocCheck(m_env.get());

	err = m_env->configure(m_envConf);
	MojErrCheck(err);

	err = m_env->open(m_databaseDir);
	MojErrCheck(err);

    // open db
    err = m_db.open(m_databaseDir.data(), m_env);
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
