find_package(PkgConfig QUIET)
PKG_CHECK_MODULES(INDB8 QUIET indb8)

find_path(INDB8_INCLUDE_DIR NAMES db/MojDb.h
   HINTS
   ${INDB8_INCLUDEDIR}
   ${INDB8_INCLUDE_DIRS}
   PATH_SUFFIXES mojodb
)

# handle the QUIETLY and REQUIRED arguments and set INDB8_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(InDB8
                                  REQUIRED_VARS INDB8_LIBRARIES INDB8_INCLUDE_DIR INDB8_CFLAGS)

mark_as_advanced(INDB8_INCLUDE_DIR INDB8_LIBRARIES INDB8_CFLAGS)
