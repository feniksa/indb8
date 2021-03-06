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

include_directories(${CMAKE_SOURCE_DIR}/inc ${CMAKE_SOURCE_DIR}/test)

# --------------------------------
# dbcore target
# ---------------------------------
set (DB_TEST_SOURCES
     MojDbTestRunner.cpp
     MojDbTestStorageEngine.cpp
)

foreach(test
        MojDbBulk
        MojDbConcurrency
        MojDbCrud
        MojDbDumpLoad
        MojDbIndex
        MojDbKind
        MojDbLocale
        MojDbPermission
        MojDbPurge
        MojDbQuery
        MojDbQueryFilter
        MojDbQuota
        MojDbRev
        MojDbRevisionSet
        MojDbSearch
        MojDbDistinct
        MojDbTextCollator
        MojDbTextTokenizer
        MojDbWatch
        MojDbWhere
        MojDbTxn
        MojDbCursorTxn
        )

        foreach (engine ${MOJO_ENGINES})
			add_test(NAME "${engine}/db/${test}" 
                     COMMAND env MOJO_ENGINE=${engine} ${CMAKE_CURRENT_BINARY_DIR}/test_db_core ${test}
            )
		endforeach()
    set(DB_TEST_SOURCES ${DB_TEST_SOURCES} ${test}Test.cpp)
endforeach()

add_executable(test_db_core ${DB_TEST_SOURCES} MojDbTestEnv.cpp)
target_link_libraries(test_db_core
                      mojocore mojodb mojodbengine
)
install( TARGETS test_db_core
    RUNTIME DESTINATION usr/share/indb8/tests
)

# --------------------------------
# performance test target
# ---------------------------------

foreach(test
     MojDbPerf
     MojDbPerfIndex
     MojDbPerfCreate
     MojDbPerfDelete
     MojDbPerfRead
     MojDbPerfUpdate
	)
	#add_test(NAME "db/perf/${test}" COMMAND test_db_performance ${test})
	set(DB_PERF_TEST_SOURCES ${DB_PERF_TEST_SOURCES} ${test}Test.cpp)
endforeach()

add_executable(test_db_performance ${DB_PERF_TEST_SOURCES} MojDbTestEnv.cpp MojDbPerfTestRunner.cpp)
target_link_libraries(test_db_performance
                      mojocore
                      mojodb
                      mojodbengine
)
install( TARGETS test_db_performance
    RUNTIME DESTINATION usr/share/indb8/tests
)

# --------------------------------
# stress test target
# ---------------------------------
set (DB_STRESS_TEST_SOURCES
     MojDbStressTest.cpp
)

add_executable(test_db_stress ${DB_STRESS_TEST_SOURCES})
target_link_libraries(test_db_stress
                      mojocore
                      mojodb
                      mojoluna
                      mojodbengine
)
install( TARGETS test_db_stress
    RUNTIME DESTINATION usr/share/indb8/tests
)
