include_directories(include)
include_directories(../pull-split/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(SOURCE_FILES pull-split-test.cpp)

set(PROJECT_NAME pull-split-test)

add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} gtest gtest_main pull-split)

if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

add_test(unit-test-pull-split ${PROJECT_NAME})
