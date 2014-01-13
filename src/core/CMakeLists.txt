set (MOJ_CORE_SOURCES
    MojApp.cpp
    MojBuffer.cpp
    MojDataSerialization.cpp
    MojDecimal.cpp
    MojEpollReactor.cpp
    MojErr.cpp
    MojFile.cpp
    MojGmainReactor.cpp
    MojHashBase.cpp
    MojJson.cpp
    MojList.cpp
    MojLog.cpp
    MojLogEngine.cpp
    MojMessageDispatcher.cpp
    MojObjectBuilder.cpp
    MojObject.cpp
    MojObjectFilter.cpp
    MojObjectSerialization.cpp
    MojOs.cpp
    MojPmLogAppender.cpp
    MojRbTreeBase.cpp
    MojRefCount.cpp
    MojSchema.cpp
    MojServiceApp.cpp
    MojService.cpp
    MojServiceMessage.cpp
    MojServiceRequest.cpp
    MojSignal.cpp
    MojSock.cpp
#    MojSocketEncoding.cpp
#    MojSocketMessage.cpp
#    MojSocketService.cpp
    MojString.cpp
    MojTestRunner.cpp
    MojThread.cpp
    MojTokenSet.cpp
    MojUtil.cpp
)

add_library(mojocore SHARED ${MOJ_CORE_SOURCES})
target_link_libraries(mojocore ${GLIB2_LDFLAGS} ${GTHREAD2_LDFLAGS} ${LS2_LDFLAGS} ${PMLOG_LDFLAGS})
install(TARGETS mojocore
        LIBRARY DESTINATION lib
)

