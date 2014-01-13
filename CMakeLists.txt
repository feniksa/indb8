# @@@LICENSE
#
# Copyright (c) 2012-2013 LG Electronics, Inc.
# Copyright (C) 2013 Maxim Sditanov
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

cmake_minimum_required(VERSION 2.8.7)

project(OBJDB CXX)

include(FindPkgConfig)

# -- check for glib 2.0
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
add_definitions(${GLIB2_CFLAGS_OTHER})

# -- check for gthread 2.0
pkg_check_modules(GTHREAD2 REQUIRED gthread-2.0)
include_directories(${GTHREAD2_INCLUDE_DIRS})
add_definitions(${GTHREAD2_CFLAGS_OTHER})

option(BUILD_LS2 "Build Mojo Luna interface" ON)

if (BUILD_LS2)
    # -- check for LS2 (internally depends on pmloglib)
    pkg_check_modules(LS2 REQUIRED luna-service2)
    include_directories(${LS2_INCLUDE_DIRS})
    add_definitions(${LS2_CFLAGS_OTHER})
endif(BUILD_LS2)

# -- check for ICU
find_library(ICU NAMES icuuc)
if(ICU STREQUAL "ICU-NOTFOUND")
	MESSAGE(FATAL_ERROR "Failed to find ICU4C libaries. Please install.")
endif()

find_library(ICUI18N NAMES icui18n)
if(ICUI18N STREQUAL "ICUI18N-NOTFOUND")
	MESSAGE(FATAL_ERROR "Failed to find ICU-I18N libaries. Please install.")
endif()

# provide both db8 and mojodb pkgconfigs
#webos_build_pkgconfig(files/pkgconfig/mojodb)
#webos_build_pkgconfig(files/pkgconfig/db8)

add_definitions(-Wall -fPIC -Wconversion -frename-registers -DMOJ_LINUX -fno-strict-aliasing -DMOJ_INTERNAL -D_REENTRANT)

# Note: no-psabi will disable notice for gcc: the mangling of 'va_list' has changed in GCC 4.4

# to turn on extra debug information (like output to log dirty buffers data) add debug compile flag -DMOJ_DEBUG_LOGGING
#webos_add_compiler_flags(DEBUG -DMOJ_DEBUG)
#webos_add_linker_options(ALL --no-undefined)

# -- add local include paths
include_directories(inc)
add_subdirectory(src/core)
add_subdirectory(src/db)

# hardcode used engine
# Todo: make ability to use any avaliable engine at runtime
add_definitions("-DMOJ_USE_LDB")
add_subdirectory(src/db-engine)

add_subdirectory(src/luna)
add_subdirectory(src/db-luna)


option(TESTS "Build tests" ON)
if (TESTS)
	enable_testing()

	add_subdirectory(test/core)
	add_subdirectory(test/db)
	add_subdirectory(test/dbclient)
	add_subdirectory(test/luna-service)

    add_subdirectory(bundled/gtest-1.6.0)
    add_subdirectory(test/mojocore)
    add_subdirectory(test/db-core)
    if (WEBOS_DB8_BACKEND STREQUAL "leveldb")
        add_subdirectory(test/leveldb)
    endif ()
    add_subdirectory(test/db-media)
endif()
#add_subdirectory(tool/dbgen)
