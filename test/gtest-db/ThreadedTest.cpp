// @@@LICENSE
//
//      Copyright (c) 2014 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// LICENSE@@@

#include "MojDbCoreTest.h"

#include <array>
#include <thread>

struct ThreadedTest : MojDbCoreTest
{
    static constexpr const char * const kindDef = "{\"id\":\"Test:1\", \"owner\":\"mojodb.admin\"}";
    void SetUp()
    {
        MojDbCoreTest::SetUp();

        // add kind
        MojObject kindObj;
        MojAssertNoErr( kindObj.fromJson(kindDef) );
        MojAssertNoErr( db.putKind(kindObj) );
    }
};

TEST_F(ThreadedTest, revision)
{
    const size_t nthreads = 8, nsteps = 10000;

    MojObject obj;
    MojAssertNoErr( obj.putString(MojDb::KindKey, "Test:1") );
    MojAssertNoErr( obj.putInt("foo", -1) );
    MojAssertNoErr( db.put(obj) );

    MojObject baseId, id;
    MojInt64 baseRev, rev;

    // put base object
    MojAssertNoErr( obj.getRequired(MojDb::IdKey, baseId) );
    MojAssertNoErr( obj.getRequired(MojDb::RevKey, baseRev) );

    // check that we can read it
    bool found = false;
    MojAssertNoErr( db.get(baseId, obj, found) );
    ASSERT_TRUE( found );
    MojAssertNoErr( obj.getRequired(MojDb::IdKey, id) );
    MojAssertNoErr( obj.getRequired(MojDb::RevKey, rev) );
    ASSERT_EQ( baseId, id );
    ASSERT_EQ( baseRev, rev );

    // put it back and notice revision bump
    MojAssertNoErr( obj.putInt("foo", -2) );
    MojAssertNoErr( db.put(obj) );
    MojAssertNoErr( obj.getRequired(MojDb::IdKey, baseId) );
    MojAssertNoErr( obj.getRequired(MojDb::RevKey, baseRev) );
    ASSERT_EQ( id, baseId );
    ASSERT_EQ( rev+1, baseRev ) << "Expected revision bump";

    // now lets check that rule in threads
    const auto f = [&](size_t worker) {
        MojObject obj;
        MojAssertNoErr( obj.putString(MojDb::KindKey, "Test:1") );
        MojAssertNoErr( obj.putInt("worker", worker) );
        for (size_t n = 0; n < nsteps; ++n)
        {
            MojAssertNoErr( obj.putInt("iteration", n) );
            MojAssertNoErr( db.put(obj) );
        }
    };

    std::array<std::thread, nthreads> threads;
    for (size_t worker = 0; worker < threads.size(); ++worker)
    {
        threads[worker] = std::thread(f, worker);
    }

    for(auto &thread : threads) thread.join();

    // put our object again to bump revision once more time and verify it
    MojAssertNoErr( obj.putInt("foo", -3) );
    MojAssertNoErr( db.put(obj) );
    MojAssertNoErr( obj.getRequired(MojDb::IdKey, id) );
    MojAssertNoErr( obj.getRequired(MojDb::RevKey, rev) );

    ASSERT_EQ( baseId, id );
    EXPECT_EQ( baseRev + MojInt64(nthreads * nsteps) + 1, rev )
        << "Expected " << nthreads * nsteps << " revision bumps from " << nthreads << " threads"
        << " and one extra bump for verification";
}
