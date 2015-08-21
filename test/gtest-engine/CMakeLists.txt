project(gtest_engine C CXX)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_definitions(-Wall -ansi -Wno-deprecated -pthread --std=c++11)

add_executable(gtest_engine EngineFactoryTest.cpp)

target_link_libraries(gtest_engine
                      mojocore
                      mojodb
                      mojodbengine
                      gtest
                      gtest_main
                     )
install( TARGETS gtest_engine
    RUNTIME DESTINATION usr/share/indb8/tests
)

add_test(NAME gtest_engine COMMAND gtest_engine)

