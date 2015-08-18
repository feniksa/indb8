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

set (LUNA_SERVICE_TEST_SOURCES
     MojLunaServiceTest.cpp
)

add_executable(test_luna_service ${LUNA_SERVICE_TEST_SOURCES})
target_link_libraries(test_luna_service
                      mojocore
                      mojodb
                      mojoluna
)
install( TARGETS test_luna_service
    RUNTIME DESTINATION usr/share/indb8/tests
)

add_test(NAME "luna-service/test_luna_service" COMMAND test_luna_service)
