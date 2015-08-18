if (BUILD_ENGINE_SANDWICH)
	add_subdirectory(sandwich)
	set (DB_BACKEND_LIB
		${DB_BACKEND_LIB}
		mojodbenginesandwich
	)
endif()
if (BUILD_ENGINE_LEVELDB)
	add_subdirectory(leveldb)
	set (DB_BACKEND_LIB
		${DB_BACKEND_LIB}
		mojodbenginelbd
	)
endif()
if (BUILD_ENGINE_BERKELEY)
	add_subdirectory(berkeley)
	set (DB_BACKEND_LIB
		${DB_BACKEND_LIB}
		mojodbenginebdb
	)
endif()

set (CXX_SOURCRES
	MojDbEngineFactory.cpp
	MojDbEngineEnvSingle.cpp
)

add_library(mojodbengine SHARED ${CXX_SOURCRES})
target_link_libraries(mojodbengine ${GLIB2_LDFLAGS} ${GTHREAD2_LDFLAGS} ${DB_BACKEND_LIB})
install(TARGETS mojodbengine
    LIBRARY DESTINATION usr/lib
)
