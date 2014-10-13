/* @@@LICENSE
*
*  Copyright (c) 2009-2014 LG Electronics, Inc.
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

#include "db-engine/sandwich/MojDbSandwichEnv.h"
#include "db-engine/sandwich/MojDbSandwichEngine.h"

const MojChar* const MojDbSandwichEnv::LockFileName = _T("_ldblock");

MojDbSandwichEnv::~MojDbSandwichEnv()
{
    close();
}

MojErr MojDbSandwichEnv::configure(const MojObject& conf)
{
    // TODO: read some configurtion

    /*
    bool found = false;
    MojString logDir;
    MojErr err = conf.get(_T("logDir"), logDir, found);
    MojErrCheck(err);
    */

    return MojErrNone;
}

MojErr MojDbSandwichEnv::open(const MojChar* dir)
{
    return lockDir(dir);
}

MojErr MojDbSandwichEnv::close()
{
    unlockDir();

    return MojErrNone;
}

MojErr MojDbSandwichEnv::lockDir(const MojChar* path)
{
    MojAssert(path);

    MojErr err = MojCreateDirIfNotPresent(path);
    MojErrCheck(err);
    err = m_lockFileName.format(_T("%s/%s"), path, LockFileName);
    MojErrCheck(err);
    err = m_lockFile.open(m_lockFileName, MOJ_O_RDWR | MOJ_O_CREAT, MOJ_S_IRUSR | MOJ_S_IWUSR);
    MojErrCheck(err);
    err = m_lockFile.lock(LOCK_EX | LOCK_NB);
    MojErrCatch(err, MojErrWouldBlock) {
        (void) m_lockFile.close();
        MojErrThrowMsg(MojErrLocked, _T("Database at '%s' locked by another instance."), path);
    }
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbSandwichEnv::unlockDir()
{

    MojErr err = MojErrNone;
    if (m_lockFile.open()) {
            // unlink before we close to ensure that we hold
        // the lock to the bitter end
        MojErr errClose = MojUnlink(m_lockFileName);
        MojErrAccumulate(err, errClose);
        errClose = m_lockFile.close();
        MojErrAccumulate(err, errClose);
    }

    return err;
}
