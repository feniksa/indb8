if (WEBOS_ENGINE_BERKELEY)
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

	set(DB_BACKEND_WRAPPER_SOURCES
		${DB_BACKEND_WRAPPER_SOURCES}
		berkeley/MojDbBerkeleyEngine.cpp
		berkeley/MojDbBerkeleyFactory.cpp
		berkeley/MojDbBerkeleyQuery.cpp
	)
	set (DB_BACKEND_LIB ${DB_BACKEND_LIB} db)
endif()
if(WEBOS_ENGINE_LEVELDB)
	message (STATUS "Use database frontend: leveldb")

	# -- check for LevelDB backend
	find_library(LDB NAMES leveldb ${WEBOS_INSTALL_ROOT}/lib)
	if(LDB STREQUAL "LDB-NOTFOUND")
		message(FATAL_ERROR "Failed to find LevelDB libaries. Please install.")
	endif()

	set(DB_BACKEND_WRAPPER_SOURCES
		${DB_BACKEND_WRAPPER_SOURCES}
		leveldb/defs.cpp
		leveldb/MojDbLevelEngine.cpp
		leveldb/MojDbLevelFactory.cpp
		leveldb/MojDbLevelDatabase.cpp
		leveldb/MojDbLevelQuery.cpp
		leveldb/MojDbLevelTxn.cpp
		leveldb/MojDbLevelSeq.cpp
		leveldb/MojDbLevelCursor.cpp
		leveldb/MojDbLevelEnv.cpp
		leveldb/MojDbLevelIndex.cpp
		leveldb/MojDbLevelItem.cpp
		leveldb/MojDbLevelTxnIterator.cpp
		leveldb/MojDbLevelIterator.cpp
		leveldb/MojDbLevelContainerIterator.cpp
	)

	set (DB_BACKEND_LIB ${DB_BACKEND_LIB} leveldb)
endif()
if (WEBOS_ENGINE_SANDWICH)
	message (STATUS "Use database frontend: sandwich")

	# -- check for LevelDB backend
	find_library(LDB NAMES leveldb ${WEBOS_INSTALL_ROOT}/lib)
	if(LDB STREQUAL "LDB-NOTFOUND")
		message(FATAL_ERROR "Failed to find LevelDB libaries. Please install.")
	endif()

	set(DB_BACKEND_WRAPPER_SOURCES
		${DB_BACKEND_WRAPPER_SOURCES}
		sandwich/MojDbSandwichEngine.cpp
		sandwich/MojDbSandwichFactory.cpp
		sandwich/MojDbSandwichDatabase.cpp
		sandwich/MojDbSandwichQuery.cpp
		sandwich/MojDbSandwichTxn.cpp
		sandwich/MojDbSandwichSeq.cpp
		sandwich/MojDbSandwichEnv.cpp
		sandwich/MojDbSandwichIndex.cpp
		sandwich/MojDbSandwichItem.cpp
		sandwich/MojDbSandwichLazyUpdater.cpp
	)
	set (DB_BACKEND_LIB ${DB_BACKEND_LIB} leveldb)
endif ()

add_library(mojodbengine SHARED ${DB_BACKEND_WRAPPER_SOURCES})
target_link_libraries(mojodbengine ${GLIB2_LDFLAGS} ${GTHREAD2_LDFLAGS} ${DB_BACKEND_LIB})
install(TARGETS mojodbengine
    LIBRARY DESTINATION usr/lib
)
