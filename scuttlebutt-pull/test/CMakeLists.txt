include_directories(include)
include_directories(../scuttlebutt-pull/scuttlebutt/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(SOURCE_FILES
        scuttlebutt/utils-test.cpp
        )

set(PROJECT_NAME scuttlebutt-pull-test)

add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} gtest gtest_main scuttlebutt-pull)

if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

add_test(unit-test-scuttlebutt-pull ${PROJECT_NAME})
