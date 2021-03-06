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


project(gtest_leveldb C CXX)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_definitions(-Wall -ansi -Wno-deprecated -pthread -std=c++11)

add_executable(gtest_leveldb
               Runner.cpp
               TestContainerIterator.cpp
               TestIterator.cpp
               TestTxn.cpp
               TestTxnIterator.cpp
)
target_link_libraries(gtest_leveldb
                      gtest
                      gtest_main
                      mojocore
                      mojodb
                      mojodbengine
                      )

install( TARGETS gtest_leveldb
    RUNTIME DESTINATION usr/share/indb8/tests
)

add_test(NAME gtest_leveldb COMMAND gtest_leveldb)
