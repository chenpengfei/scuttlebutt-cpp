include_directories(include)
include_directories(../socket-stream/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(SOURCE_FILES socket-stream-test.cpp)

set(PROJECT_NAME socket-stream-test)

add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} gtest gtest_main socket-stream)

if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

add_test(unit-test-socket-stream ${PROJECT_NAME})
