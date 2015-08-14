# @@@LICENSE
#
# Copyright (C) 2014-2015 Maksym Sditanov
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

project(INDB8 CXX)

# Build options
option(BUILD_LS2             "Build Mojo Luna interface" ON )
option(BUILD_TESTING         "Build tests"               ON )
option(BUILD_TOOL_JSONOBFUSCATE "Build obfuscate tool"   OFF)
option(BUILD_ENGINE_SANDWICH "Build sandwich engine"     ON )
option(BUILD_ENGINE_LEVELDB  "Build leveldb  engine"     OFF)
option(BUILD_ENGINE_BERKELEY "Build berkeley engine"     OFF)

include(FindPkgConfig)

add_definitions(-std=c++11 -Wall -fPIC -Wconversion -frename-registers -DMOJ_LINUX -fno-strict-aliasing -DMOJ_INTERNAL -D_REENTRANT)

# -- check for glib 2.0
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
add_definitions(${GLIB2_CFLAGS_OTHER})

# -- check for gthread 2.0
pkg_check_modules(GTHREAD2 REQUIRED gthread-2.0)
include_directories(${GTHREAD2_INCLUDE_DIRS})
add_definitions(${GTHREAD2_CFLAGS_OTHER})

# -- check for ICU
find_library(ICU NAMES icuuc)
if(ICU STREQUAL "ICU-NOTFOUND")
	MESSAGE(FATAL_ERROR "Failed to find ICU4C libaries. Please install.")
endif()

find_library(ICUI18N NAMES icui18n)
if(ICUI18N STREQUAL "ICUI18N-NOTFOUND")
	MESSAGE(FATAL_ERROR "Failed to find ICU-I18N libaries. Please install.")
endif()

if (BUILD_LS2)
	# -- check for LS2 (internally depends on pmloglib)
	pkg_check_modules(LS2 REQUIRED luna-service2)
	include_directories(${LS2_INCLUDE_DIRS})
	add_definitions(${LS2_CFLAGS_OTHER})
endif()

if (BUILD_ENGINE_SANDWICH)
	add_definitions(-DMOJ_USE_SANDWICH)
endif()
if (BUILD_ENGINE_LEVELDB)
	add_definitions(-DMOJ_USE_LDB)
endif()
if (BUILD_ENGINE_BERKELEY)
	add_definitions(-DMOJ_USE_BDB)
endif()


#add_definitions("-std=c++11"
#                "-fno-elide-constructors"
#               "-pedantic-errors"
#                "-Wextra"
#                "-Wall"
#                "-Winit-self"
#                "-Wold-style-cast"
#                "-Woverloaded-virtual"
#                "-Wuninitialized"
#                "-Wmissing-declarations"
#                "-Winit-self")


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMOJ_DEBUG -DMOJ_DEBUG_LOGGING")

if (NOT BUILD_ENGINE_SANDWICH AND NOT BUILD_ENGINE_LEVELDB AND NOT BUILD_ENGINE_BERKELEY)
	message(FATAL_ERROR "Not selected any of engines")
endif()

# -- add local include paths
include_directories(inc)

add_subdirectory(src/core)
add_subdirectory(src/db)
add_subdirectory(src/db-engine)

function(install_ls2_roles path role)
	file(GLOB files "${path}/*.json.${role}.in")
	foreach(file ${files})
		get_filename_component(filename ${file} NAME)
		string(REGEX REPLACE ".${role}.in" "" dst_filename ${filename})
		configure_file(${file} "files/sysbus/${role}/${dst_filename}")
		install(FILES "${CMAKE_CURRENT_BINARY_DIR}/files/sysbus/${role}/${dst_filename}" DESTINATION "usr/share/ls2/roles/${role}")
	endforeach()
endfunction()

function(install_ls2_service path role)
	file(GLOB files "${path}/*.service.${role}.in")
	foreach(file ${files})
		get_filename_component(filename ${file} NAME)
		string(REGEX REPLACE ".${role}.in" "" dst_filename ${filename})
		configure_file(${file} "files/sysbus/${role}/${dst_filename}")
		install(FILES "${CMAKE_CURRENT_BINARY_DIR}/files/sysbus/${role}/${dst_filename}" DESTINATION "usr/share/dbus-1/system-services")
	endforeach()
endfunction()

if (BUILD_LS2)
	add_subdirectory(src/luna)
	add_subdirectory(src/db-luna)

	install_ls2_roles("files/sysbus" "prv")
	install_ls2_roles("files/sysbus" "pub")
	install_ls2_service("files/sysbus" "prv")
	install_ls2_service("files/sysbus" "pub")
endif()

if (BUILD_TESTING)
	enable_testing()

	add_subdirectory(test/core)
	add_subdirectory(test/db)
	add_subdirectory(test/gtest-core)
	add_subdirectory(test/gtest-db)
	if (WEBOS_ENGINE_LEVELDB)
		add_subdirectory(test/gtest-leveldb)
	endif ()
	if (BUILD_LS2)
		add_subdirectory(test/luna-service)

		install_ls2_roles("test/files/sysbus" "prv")
		install_ls2_roles("test/files/sysbus" "pub")
		install_ls2_service("test/files/sysbus" "prv")
		install_ls2_service("test/files/sysbus" "pub")

		add_subdirectory(test/dbclient)
	endif()
endif()

install(FILES files/conf/maindb.conf DESTINATION etc/indb8)
install(FILES files/conf/tempdb.conf DESTINATION etc/indb8)

configure_file(files/pkgconfig/indb8.pc.in
	${CMAKE_CURRENT_BINARY_DIR}/usr/lib/pkgconfig/indb8.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/usr/lib/pkgconfig/indb8.pc
	    DESTINATION usr/lib/pkgconfig/indb8.pc)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/usr/lib/pkgconfig/indb8.pc
	    DESTINATION usr/lib/pkgconfig/db8.pc)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/usr/lib/pkgconfig/indb8.pc
	    DESTINATION usr/lib/pkgconfig/mojodb.pc)


install(DIRECTORY inc DESTINATION usr/include/mojodb)

if (BUILD_TOOL_JSONOBFUSCATE)
	message(STATUS "Build json obfuscate tool")
	add_subdirectory(tool/JsonObfuscator)
endif()
