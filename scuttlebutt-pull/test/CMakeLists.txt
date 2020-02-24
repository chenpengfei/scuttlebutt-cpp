include_directories(./include
        ../scuttlebutt-pull/duplex/include
        ../scuttlebutt-pull/model/include
        ../scuttlebutt-pull/scuttlebutt/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(SOURCE_FILES
        src/stream-events-test.cpp
        src/utils-test.cpp)

set(PROJECT_NAME scuttlebutt-pull-test)

add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} gtest gtest_main scuttlebutt-pull monotonic-timestamp)

if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

add_test(unit-test-scuttlebutt-pull ${PROJECT_NAME})
