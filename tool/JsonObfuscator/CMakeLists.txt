project(mojoobfuscate CXX)

# -- Check for Boost library
find_package(Boost 1.40 COMPONENTS regex REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

set (CXX_SOURCRES
	JsonObfuscator.cpp
	main.cpp
)

add_executable(mojoobfuscate ${CXX_SOURCRES})
target_link_libraries(mojoobfuscate ${Boost_LIBRARIES})

install( TARGETS mojoobfuscate
    RUNTIME DESTINATION usr/bin
)
