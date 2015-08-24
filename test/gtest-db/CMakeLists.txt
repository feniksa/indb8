# @@@LICENSE
#
# Copyright (c) 2013-2015 LG Electronics, Inc.
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


project(gtest_db C CXX)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../db)

add_definitions(-Wall -ansi -Wno-deprecated -pthread --std=c++11)

foreach(test
		Batch
		DumpLoad
		Req
		Quota
		Search
		TextCollator
		Kinds
	)
	foreach (engine ${MOJO_ENGINES})
		add_test(NAME "${engine}/gtest-db/${test}"
		         COMMAND env MOJO_ENGINE=${engine} ${CMAKE_CURRENT_BINARY_DIR}/gtest_db --gtest_filter="${test}Test.*")
	endforeach()

	set(TEST_SOURCES ${TEST_SOURCES} ${test}Test.cpp)
endforeach()

add_executable(gtest_db  ${TEST_SOURCES} ../db/MojDbTestStorageEngine.cpp Runner.cpp MojDbCoreTest.cpp)

target_link_libraries(gtest_db
	mojocore
	mojodb
	mojodbengine
	gtest
	gtest_main
)

install( TARGETS gtest_db
    RUNTIME DESTINATION usr/share/indb8/tests
)
