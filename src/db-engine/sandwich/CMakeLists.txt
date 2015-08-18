message (STATUS "Use database frontend: sandwich")

# -- check for LevelDB backend
find_library(LDB NAMES leveldb ${WEBOS_INSTALL_ROOT}/lib)
if(LDB STREQUAL "LDB-NOTFOUND")
	message(FATAL_ERROR "Failed to find LevelDB libaries. Please install.")
endif()

include_directories(${CMAKE_SOURCE_DIR}/inc)

set(CXX_SOURCES
	MojDbSandwichEngine.cpp
	MojDbSandwichFactory.cpp
	MojDbSandwichDatabase.cpp
	MojDbSandwichQuery.cpp
	MojDbSandwichTxn.cpp
	MojDbSandwichSeq.cpp
	MojDbSandwichEnv.cpp
	MojDbSandwichIndex.cpp
	MojDbSandwichItem.cpp
	MojDbSandwichLazyUpdater.cpp
)

add_library(mojodbenginesandwich STATIC ${CXX_SOURCES})

# TODO make include by path, not hardcode lib
target_link_libraries(mojodbenginesandwich ${GLIB2_LDFLAGS} ${GTHREAD2_LDFLAGS} leveldb)
