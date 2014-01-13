# @@@LICENSE
#
# Copyright (c) 2013 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# LICENSE@@@


project(gtest_db_core C CXX)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../db)

add_definitions(-Wall -ansi -Wno-deprecated -pthread --std=c++0x)

add_executable(gtest_db_core
               Runner.cpp
               JsonTest.cpp
               DumpLoadTest.cpp
               ReqTest.cpp
               QuotaTest.cpp
               SearchTest.cpp
               TextCollatorTest.cpp
               IdGeneratorTest.cpp
               ShardsTest.cpp
               ../db/MojDbTestStorageEngine.cpp
               )

target_link_libraries(gtest_db_core
                      mojocore
                      mojodb
                      mojodbengine
                      gtest
                      gtest_main
                     )
install( TARGETS gtest_db_core
    RUNTIME DESTINATION lib/indb8/tests
)
                     
add_test(NAME gtest_db_core COMMAND gtest_db_core)

