/* @@@LICENSE
*
* Copyright (c) 2009-2013 LG Electronics, Inc.
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


#include "db-luna/MojDbLunaServiceApp.h"
#include "db-luna/MojDbServiceHandler.h"
#include "db/MojDbEnv.h"
#include "db/MojDbServiceDefs.h"

#ifdef MOJ_USE_BDB
#include "db-luna/MojDbBerkeleyFactory.h"
#elif MOJ_USE_LDB
#include "db-engine/leveldb/MojDbLevelFactory.h"
#elif MOJ_USE_SANDWICH
#include "db-engine/sandwich/MojDbSandwichFactory.h"
#else
#error "Set database type"
#endif

#ifndef MOJ_VERSION_STRING
#define MOJ_VERSION_STRING NULL
#endif

const MojChar* const MojDbLunaServiceApp::VersionString = MOJ_VERSION_STRING;

int main(int argc, char** argv)
{
   MojAutoPtr<MojDbLunaServiceApp> app(new MojDbLunaServiceApp);
   MojAllocCheck(app.get());
   return app->main(argc, argv);
}

MojDbLunaServiceApp::MojDbLunaServiceApp()
: MojReactorApp<MojGmainReactor>(MajorVersion, MinorVersion, VersionString)
, m_mainService(m_dispatcher)
{
   // set up db first
#ifdef MOJ_USE_BDB
   MojDbStorageEngine::setEngineFactory(new MojDbBerkeleyFactory());
#elif MOJ_USE_LDB
   MojDbStorageEngine::setEngineFactory(new MojDbLevelFactory());
#elif MOJ_USE_SANDWICH
   MojDbStorageEngine::setEngineFactory(new MojDbSandwichFactory());
#else
  #error "Database not set"
#endif
   MojLogTrace(s_log);
}

MojDbLunaServiceApp::~MojDbLunaServiceApp()
{
	MojLogTrace(s_log);
}

MojErr MojDbLunaServiceApp::init()
{
	MojLogTrace(s_log);

	MojErr err = Base::init();
	MojErrCheck(err);

    MojDbStorageEngine::createEnv(m_env);
    MojAllocCheck(m_env.get());

    m_internalHandler.reset(new MojDbServiceHandlerInternal(m_mainService.db(), m_reactor, m_mainService.service()));
    MojAllocCheck(m_internalHandler.get());

    err = m_mainService.init(m_reactor);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServiceApp::configure(const MojObject& conf)
{
	MojLogTrace(s_log);

	MojErr err = Base::configure(conf);
	MojErrCheck(err);

    MojObject engineConf;

	conf.get(MojDbStorageEngine::engineFactory()->name(), engineConf);
	err = m_env->configure(engineConf);
    MojErrCheck(err);
    err = m_mainService.db().configure(conf);
    MojErrCheck(err);
	m_conf = engineConf;

	MojObject dbConf;
	err = conf.getRequired("db", dbConf);
	MojErrCheck(err);

	MojObject dbPath;
	err = dbConf.getRequired("path", dbPath);
	MojErrCheck(err);
	err = dbPath.stringValue(m_dbDir);
	MojErrCheck(err);

	MojObject serviceName;
	err = dbConf.getRequired("service_name", serviceName);
	MojErrCheck(err);
	err = serviceName.stringValue(m_serviceName);
	MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServiceApp::open()
{
	MojLogTrace(s_log);
	MojLogDebug(s_log, _T("mojodb starting..."));

	MojErr err = Base::open();
	MojErrCheck(err);

	// start message queue thread pool
	err = m_dispatcher.start(NumThreads);
	MojErrCheck(err);

	// open db env
	bool dbOpenFailed = false;
	err = m_env->open(m_dbDir);
	MojErrCatchAll(err) {
		dbOpenFailed = true;
	}

	// open db services
	err = m_mainService.open(m_reactor, m_env.get(), m_serviceName, m_dbDir, m_conf);
	MojErrCatchAll(err) {
		dbOpenFailed = true;
	}

	// open internal handler
	err = m_internalHandler->open();
	MojErrCheck(err);
	err = m_mainService.service().addCategory(MojDbServiceDefs::InternalCategory, m_internalHandler.get());
	MojErrCheck(err);
	err = m_internalHandler->configure(dbOpenFailed);
	MojErrCheck(err);

	MojLogDebug(s_log, _T("mojodb started"));

	return MojErrNone;
}

MojErr MojDbLunaServiceApp::close()
{
	MojLogTrace(s_log);
	MojLogDebug(s_log, _T("mojodb stopping..."));

	// stop dispatcher
	MojErr err = MojErrNone;
	MojErr errClose = m_dispatcher.stop();
	MojErrAccumulate(err, errClose);
	errClose = m_dispatcher.wait();
	MojErrAccumulate(err, errClose);
	// close services
	errClose = m_mainService.close();
	MojErrAccumulate(err, errClose);

	m_internalHandler->close();
	m_internalHandler.reset();

	errClose = Base::close();
	MojErrAccumulate(err, errClose);

	MojLogDebug(s_log, _T("mojodb stopped"));

	return err;
}

MojErr MojDbLunaServiceApp::handleArgs(const StringVec& args)
{
	MojErr err = Base::handleArgs(args);
	MojErrCheck(err);

	MojLogDebug(s_log, _T("Pathed arguments: %li"), args.size());

	return MojErrNone;
}

MojErr MojDbLunaServiceApp::displayUsage()
{
	MojErr err = displayMessage(_T("Usage: %s [OPTION]...\n"), name().data());
	MojErrCheck(err);

	return MojErrNone;
}
