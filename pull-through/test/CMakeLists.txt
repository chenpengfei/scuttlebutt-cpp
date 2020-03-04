include_directories(include)
include_directories(../pull-through/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(SOURCE_FILES pull-through-test.cpp)

set(PROJECT_NAME pull-through-test)

add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} gtest gtest_main pull-through)

if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

add_test(unit-test-pull-through ${PROJECT_NAME})
