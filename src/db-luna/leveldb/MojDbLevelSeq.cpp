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

#include "db-luna/leveldb/MojDbLevelSeq.h"
#include "db-luna/leveldb/MojDbLevelDatabase.h"
#include "db-luna/leveldb/MojDbLevelEngine.h"

// at this point it's just a placeholder
MojDbLevelSeq::~MojDbLevelSeq()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    if (m_db) {
        MojErr err = close();
        MojErrCatchAll(err);
    }
}

MojErr MojDbLevelSeq::open(const MojChar* name, MojDbLevelDatabase* db)
{
    MojAssert(db);
    MojLogTrace(MojDbLevelEngine::s_log);

    //MojAssert(!m_seq);
    MojErr err;

    m_db = db;
    err = m_key.fromBytes(reinterpret_cast<const MojByte  *>(name), strlen(name));
    MojErrCheck(err);

    MojDbLevelItem val;
    bool found = false;
    err = m_db->get(m_key, NULL, false, val, found);
    MojErrCheck(err);

    if (found)
    {
        MojObject valObj;
        err = val.toObject(valObj);
        MojErrCheck(err);
        m_next = valObj.intValue();
    }
    else
    {
        m_next = 0;
    }

    m_allocated = m_next;

    return MojErrNone;
}

MojErr MojDbLevelSeq::close()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    if (m_db) {
        MojErr err = store(m_next);
        MojErrCheck(err);
        m_db = NULL;
    }
    return MojErrNone;
}

MojErr MojDbLevelSeq::get(MojInt64& valOut)
{
    MojLogTrace(MojDbLevelEngine::s_log);

    if (m_next == m_allocated)
    {
        MojErr err = MojDbLevelSeq::allocateMore();
        MojErrCheck(err);
        MojAssert(m_allocated > m_next);
    }
    valOut = m_next++;

    return MojErrNone;
}

MojErr MojDbLevelSeq::allocateMore()
{
    return store(m_allocated + 100);
}

MojErr MojDbLevelSeq::store(MojInt64 next)
{
    MojAssert( next >= m_next );
    MojErr err;
    MojDbLevelItem val;

#ifdef MOJ_DEBUG
    // ensure that our state is consistent with db
    bool found = false;
    err = m_db->get(m_key, NULL, false, val, found);
    MojErrCheck(err);

    if (found)
    {
        MojObject valObj;
        err = val.toObject(valObj);
        MojErrCheck(err);
        if (m_allocated != valObj.intValue()) return MojErrDbInconsistentIndex;
    }
    else
    {
        if (m_allocated != 0) return MojErrDbInconsistentIndex;
    }

#endif

    err = val.fromObject(next);
    MojErrCheck(err);
    err = m_db->put(m_key, val, NULL, false);
    MojErrCheck(err);
    m_allocated = next;

    return MojErrNone;
}


