message (STATUS "Use database frontend: berkeley")
# -- check for BerkeleyDB
# (add an alternate standard path in case building BDB locally: does not override)
find_library(BDB NAMES db-4.8)
if(BDB STREQUAL "BDB-NOTFOUND")
	message(FATAL_ERROR "Failed to find BerkleyDB libaries. Please install.")
endif()

find_path(BDB_INC db.h
			PATHS /usr/local/BerkeleyDB.4.8/include
			PATH_SUFFIXES db4.8)
if(BDB_INC STREQUAL "BDB_INC-NOTFOUND")
	message(FATAL_ERROR "Failed to find BerkleyDB includes. Please install.")
endif()

include_directories(${CMAKE_SOURCE_DIR}/inc)

set(CXX_SOURCES
	MojDbBerkeleyCursor.cpp
	MojDbBerkeleyDatabase.cpp
	MojDbBerkeleyDefs.cpp
	MojDbBerkeleyEngine.cpp
	MojDbBerkeleyEnv.cpp
	MojDbBerkeleyIndex.cpp
	MojDbBerkeleyItem.cpp
	MojDbBerkeleySeq.cpp
	MojDbBerkeleyTxn.cpp
	MojDbBerkeleyFactory.cpp
	MojDbBerkeleyQuery.cpp
)
add_library(mojodbenginebdb STATIC ${CXX_SOURCES})

# TODO make include by path, not hardcode lib
target_link_libraries(mojodbenginebdb ${GLIB2_LDFLAGS} ${GTHREAD2_LDFLAGS} db)
