include_directories(../duplex-pull/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(SOURCE_FILES duplex-pull-test.cpp)

set(PROJECT_NAME duplex-pull-test)

add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} gtest gtest_main duplex-pull)

if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

add_test(unit-test-duplex-pull ${PROJECT_NAME})
