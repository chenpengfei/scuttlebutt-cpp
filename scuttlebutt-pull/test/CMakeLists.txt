include_directories(./include
        ../scuttlebutt-pull/duplex/include
        ../scuttlebutt-pull/model/include
        ../scuttlebutt-pull/scuttlebutt/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(SOURCE_FILES
        src/model-test.cpp
        src/model-filter-test.cpp
        src/stream-end-test.cpp
        src/stream-events-test.cpp
        src/utils-test.cpp)

set(PROJECT_NAME scuttlebutt-pull-test)

add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} gtest gtest_main scuttlebutt-pull)

if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

add_test(unit-test-scuttlebutt-pull ${PROJECT_NAME})
