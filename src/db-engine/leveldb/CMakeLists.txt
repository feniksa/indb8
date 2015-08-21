message (STATUS "Use database frontend: leveldb")

# -- check for LevelDB backend
find_library(LDB NAMES leveldb ${WEBOS_INSTALL_ROOT}/lib)
if(LDB STREQUAL "LDB-NOTFOUND")
	message(FATAL_ERROR "Failed to find LevelDB libaries. Please install.")
endif()

include_directories(${CMAKE_SOURCE_DIR}/inc)

set(CXX_SOURCES
	defs.cpp
	MojDbLevelEngine.cpp
	MojDbLevelFactory.cpp
	MojDbLevelDatabase.cpp
	MojDbLevelQuery.cpp
	MojDbLevelTxn.cpp
	MojDbLevelSeq.cpp
	MojDbLevelCursor.cpp
	MojDbLevelEnv.cpp
	MojDbLevelIndex.cpp
	MojDbLevelItem.cpp
	MojDbLevelTxnIterator.cpp
	MojDbLevelIterator.cpp
	MojDbLevelContainerIterator.cpp
)

add_library(mojodbenginelbd STATIC ${CXX_SOURCES})

# TODO make include by path, not hardcode lib
target_link_libraries(mojodbenginelbd ${GLIB2_LDFLAGS} ${GTHREAD2_LDFLAGS} leveldb)

