set(MOJ_DB_SOURCES
    MojDbAdmin.cpp
    MojDbClient.cpp
    MojDb.cpp
    MojDbCursor.cpp
    MojDbExtractor.cpp
    MojDbIdGenerator.cpp
    MojDbIndex.cpp
    MojDbIsamQuery.cpp
    MojDbKey.cpp
    MojDbKind.cpp
    MojDbKindEngine.cpp
    MojDbKindIdList.cpp
    MojDbKindState.cpp
    MojDbObjectHeader.cpp
    MojDbObjectItem.cpp
    MojDbPermissionEngine.cpp
    MojDbPutHandler.cpp
    MojDbQuery.cpp
    MojDbQueryFilter.cpp
    MojDbQueryParser.cpp
    MojDbQueryPlan.cpp
    MojDbQuotaEngine.cpp
    MojDbReq.cpp
    MojDbRevisionSet.cpp
    MojDbSearchCursor.cpp
    MojDbServiceClient.cpp
    MojDbServiceDefs.cpp
    MojDbStorageEngine.cpp
    MojDbTextCollator.cpp
    MojDbTextTokenizer.cpp
    MojDbTextUtils.cpp
    MojDbUtils.cpp
    MojDbWatcher.cpp
)

add_library(mojodb SHARED ${MOJ_DB_SOURCES})
target_link_libraries(mojodb
                      ${GLIB2_LDFLAGS}
                      ${GTHREAD2_LDFLAGS}
                      mojocore
                      ${LS2_LDFLAGS}
                      ${ICU}
                      ${ICUI18N}
)
install(TARGETS mojodb
        LIBRARY DESTINATION usr/lib
)
