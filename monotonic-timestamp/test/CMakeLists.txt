include_directories(../monotonic-timestamp/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(SOURCE_FILES timestamp-test.cpp)

set(PROJECT_NAME monotonic-timestamp-test)

add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} gtest gtest_main monotonic-timestamp)

if (${ENABLE_COVERAGE})
  target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

add_test(unit-test-monotonic-timestamp ${PROJECT_NAME})
