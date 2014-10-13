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

/**
 *  @file IdGeneratorTest.cpp
 *  Verify MojDbIdGenerator logic
 */

#include "Runner.h"

#include <core/MojObject.h>
#include <db/MojDbIdGenerator.h>

namespace {
    const size_t ShortIdChars = 11u;
    const size_t LongIdChars = 16u;
    const MojUInt32 MagicId = 42u;
}

/**
 * Test fixture for tests around MojDbIdGenerator
 */
struct IdGeneratorTest : public ::testing::Test
{
    MojDbIdGenerator gen;

    void SetUp()
    {
        gen.init();
    }

    void genId(MojString &id)
    {
        MojObject idObj;
        MojAssertNoErr( gen.id(idObj) );

        ASSERT_EQ( MojObject::TypeString, idObj.type() );
        MojAssertNoErr( idObj.stringValue(id) );
    }

    template <typename T>
    void genId(MojString &id)
    {
        MojObject idObj;
        MojAssertNoErr( gen.id(idObj) );

        ASSERT_EQ( MojObject::TypeString, idObj.type() );
        MojAssertNoErr( idObj.stringValue(id) );
    }
};

/**
 * @test Test generation of short _id when shard genId is set to main shard or
 * omitted In this case length should be ceil(64/8 * 4/3) = 11
 */
TEST_F(IdGeneratorTest, shortId_length)
{
    MojString x;

    genId(x);
    EXPECT_EQ( ShortIdChars, x.length() );
}

/**
 * @test Test generation of short _id when shard genId is not a main shard In this
 * case length should be ceil((32+64)/8 * 4/3) = 16
 */
TEST_F(IdGeneratorTest, longId_length)
{
    MojString x;

    genId(x);
    EXPECT_EQ( LongIdChars, x.length() );
    printf(">> %s\n", x.begin());
}