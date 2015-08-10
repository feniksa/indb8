project(mojoobfuscate CXX)

# -- Check for Boost library
find_package(Boost 1.40 COMPONENTS regex REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

set (CXX_SOURCRES
	JsonObfuscator.cpp
)

add_executable(${PROJECT_NAME} ${CXX_SOURCRES})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

install( TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION usr/bin
)

add_test(NAME gtest_core COMMAND gtest_core)
