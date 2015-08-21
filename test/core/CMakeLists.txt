# @@@LICENSE
#
# Copyright (c) 2013-2015 LG Electronics
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

foreach(test
     MojAtomicInt
     MojAutoPtr
     MojBuffer
     MojDataSerialization
     MojDecimal
     MojErr
     MojHashMap
     MojJson
     MojList
     MojLog
     MojMap
     MojMessageDispatcher
     MojObjectFilter
     MojObjectSerialization
     MojObject
     MojReactor
     MojRefCount
     MojSchema
     MojSet
     MojSignal
     MojString
     MojThread
     MojTime
     MojUtil
     MojVector
)

    add_test(NAME "core/${test}" COMMAND test_core ${test})
    set(TEST_SOURCES ${TEST_SOURCES} ${test}Test.cpp)
endforeach()

add_executable(test_core ${TEST_SOURCES} MojCoreTestRunner.cpp)
target_link_libraries(test_core mojocore)

install( TARGETS test_core
    RUNTIME DESTINATION usr/share/indb8/tests
)
