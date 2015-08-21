if (BUILD_ENGINE_SANDWICH)
	add_subdirectory(sandwich)
	set (DB_BACKEND_LIB
		${DB_BACKEND_LIB}
		mojodbenginesandwich
	)
	set (ENGINES ${ENGINES} sandwich)
	add_definitions(-DBUILD_ENGINE_SANDWICH)
endif()
if (BUILD_ENGINE_LEVELDB)
	add_subdirectory(leveldb)
	set (DB_BACKEND_LIB
		${DB_BACKEND_LIB}
		mojodbenginelbd
	)
	set (ENGINES ${ENGINES} leveldb)
	add_definitions(-DBUILD_ENGINE_LEVELDB)
endif()
if (BUILD_ENGINE_BERKELEY)
	add_subdirectory(berkeley)
	set (DB_BACKEND_LIB
		${DB_BACKEND_LIB}
		mojodbenginebdb
	)
	set (ENGINES ${ENGINES} berkeley)
	add_definitions(-DBUILD_ENGINE_BERKELEY)
endif()

set (CXX_SOURCRES
	MojDbEngineFactory.cpp
)

set (MOJO_ENGINES ${ENGINES} PARENT_SCOPE)

add_library(mojodbengine SHARED ${CXX_SOURCRES})
target_link_libraries(mojodbengine mojocore mojodb ${DB_BACKEND_LIB})
install(TARGETS mojodbengine
    LIBRARY DESTINATION usr/lib
)
