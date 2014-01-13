# -- source for generating mojodb-luna
set(LUNA_BIN_SOURCES
    MojDbLunaServiceApp.cpp
    MojDbLunaServiceDb.cpp
    MojDbLunaSpaceAlert.cpp
    MojDbMediaHandler.cpp
    MojDbServiceHandler.cpp
    MojDbServiceHandlerBase.cpp
    MojDbServiceHandlerInternal.cpp
    MojDbServiceSchemas.cpp
)

include_directories (${DB_BACKEND_INCLUDES})
add_executable(mojodb-luna ${LUNA_BIN_SOURCES} ${DB_BACKEND_WRAPPER_SOURCES})
target_link_libraries(mojodb-luna
                      ${GLIB2_LDFLAGS}
                      ${GTHREAD2_LDFLAGS}
                      ${LS2_LDFLAGS}
                      ${ICU}
                      ${ICUI18N}
                      ${DB_BACKEND_LIB}
                      mojocore
                      mojodb
                      mojodbengine
                      mojoluna
                      leveldb       # hardcoded, should be removed
)

install(TARGETS mojodb-luna 
        RUNTIME DESTINATION bin
)
