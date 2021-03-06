/* @@@LICENSE
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
* LICENSE@@@ */

#ifndef MOJDBLEVELENV_H
#define MOJDBLEVELENV_H

#include <leveldb/db.h>
#include "db/MojDbDefs.h"
#include "db/MojDbEnv.h"
#include "core/MojFile.h"
#include "core/MojString.h"

class MojDbLevelEnv : public MojDbEnv
{
public:
    static MojLogger s_log;

    MojDbLevelEnv();
    ~MojDbLevelEnv();

    MojErr configure(const MojObject& conf) override;
    MojErr open(const MojChar* path) override;
    MojErr close() override;
	MojErr openStorage(MojRefCountedPtr<MojDbStorageEngine>& storage) override;

    MojErr postCommit(MojSize updateSize);

    void* impl() { return m_db; }
    static MojErr translateErr(int dbErr);

private:
    static const MojChar* const LockFileName;

    MojErr lockDir(const MojChar* path);
    MojErr unlockDir();

    MojString m_lockFileName;
    MojFile m_lockFile;
    MojString m_logDir;
    leveldb::DB* m_db;
};

#endif

