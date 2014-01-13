set (MOJ_LUNA_SOURCES
    MojLunaErr.cpp
    MojLunaMessage.cpp
    MojLunaRequest.cpp
    MojLunaService.cpp
)

add_library(mojoluna SHARED ${MOJ_LUNA_SOURCES})
target_link_libraries(mojoluna
                      ${GLIB2_LDFLAGS}
                      ${GTHREAD2_LDFLAGS}
                      ${LS2_LDFLAGS}
                      ${ICU}
                      ${ICUI18N}
                      mojocore
)

install(TARGETS mojoluna
        LIBRARY DESTINATION usr/lib
)

