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


project(gtest_core CXX)

add_definitions(-Wall -std=c++0x -Wno-deprecated -pthread)
add_definitions(-DDEFAULT_SETTINGS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/defaultSettingsObfuscated.json")

foreach(test
		AtomicInt
		Decimal
		Number
		Json
        Signal
	)
	add_test(NAME "gtest-core/${test}" COMMAND gtest_core --gtest_filter="${test}.*")
	set(TEST_SOURCES ${TEST_SOURCES} ${test}Test.cpp)
endforeach()

add_executable(gtest_core ${TEST_SOURCES} Runner.cpp)
target_link_libraries(gtest_core gtest gtest_main mojocore)

install( TARGETS gtest_core
	RUNTIME DESTINATION usr/share/indb8/tests
)
