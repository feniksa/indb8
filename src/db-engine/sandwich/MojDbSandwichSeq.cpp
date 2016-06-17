/* @@@LICENSE
*
* Copyright (c) 2013-2014 LG Electronics, Inc.
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

#include "db-engine/sandwich/MojDbSandwichSeq.h"
#include "db-engine/sandwich/MojDbSandwichDatabase.h"
#include "db-engine/sandwich/MojDbSandwichEngine.h"

namespace {
    constexpr MojInt64 SequenceAllocationPage = 100;
} // anonymous namespace

// at this point it's just a placeholder
MojDbSandwichSeq::~MojDbSandwichSeq()
{
    if (m_db) {
        MojErr err = close();
        MojErrCatchAll(err);
    }
}

MojErr MojDbSandwichSeq::open(const MojChar* name, MojDbSandwichDatabase* db)
{
    MojAssert(db);

    //MojAssert(!m_seq);
    MojErr err;

    m_db = db;
    err = m_key.fromBytes(reinterpret_cast<const MojByte  *>(name), strlen(name));
    MojErrCheck(err);

    MojDbSandwichItem val;
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

    m_allocated.store(m_next, std::memory_order_relaxed);

    return MojErrNone;
}

MojErr MojDbSandwichSeq::close()
{
    if (m_db) {
        MojErr err = store(m_next);
        MojErrCheck(err);
        m_db = NULL;
    }
    return MojErrNone;
}

MojErr MojDbSandwichSeq::get(MojInt64& valOut)
{
#ifdef MOJ_DEBUG
    valOut = -1; // in case of error trace down those who do not check MojErr
#endif

    auto val = m_next.fetch_add(1, std::memory_order_relaxed); // only attomicity of increment required

    // the only stuff we need here is atomicity of load()
    // we are ok if some concurrent get() will fallback to mutex due due to
    // read of outdated value
    if (val >= m_allocated.load(std::memory_order_relaxed))
    {
        // everyone who received val from unallocated range will come here and
        // will be processed in a queue on this mutex
        std::lock_guard<std::mutex> allocationGuard(m_allocationLock);

        // re-read under lock before allocating more sequence values
        auto allocated = m_allocated.load(std::memory_order_relaxed);
        if (val >= allocated) // no other threads who marked our value allocated
        {
            // mark our allocated val as a start of next pre-allocated page
            MojErr err = store(val + SequenceAllocationPage);
            MojErrCheck(err);
        }
    }
    valOut = (MojInt64)val;

    return MojErrNone;
}

MojErr MojDbSandwichSeq::store(MojUInt64 next)
{
    MojAssert( next >= m_next );
    MojErr err;
    MojDbSandwichItem val;

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
        if (m_allocated != MojUInt64(valObj.intValue()))
            MojErrThrowMsg(MojErrDbInconsistentIndex, "Stored and cached seq are different");
    }
    else
    {
        if (m_allocated != 0)
            MojErrThrowMsg(MojErrDbInconsistentIndex, "Failed to fetch previously allocated seq");
    }

#endif

    err = val.fromObject(MojInt64(next));
    MojErrCheck(err);
    err = m_db->put(m_key, val, NULL, false);
    MojErrCheck(err);
    m_allocated.store(next, std::memory_order_relaxed);

    return MojErrNone;
}
