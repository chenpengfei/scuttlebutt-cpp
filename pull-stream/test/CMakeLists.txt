include_directories(include)
include_directories(../pull-stream/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(SOURCE_FILES
        src/pull-test.cpp
        src/map-test.cpp
        src/values-test.cpp
        )

set(PROJECT_NAME pull-stream-test)

add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} gtest gtest_main pull-looper)

if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

add_test(unit-test-pull-stream ${PROJECT_NAME})
