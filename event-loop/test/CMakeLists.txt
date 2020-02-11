include_directories(../event-loop/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(SOURCE_FILES event-loop-test.cpp)

set(PROJECT_NAME event-loop-test)

add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} gtest gtest_main event-loop)

if (${ENABLE_COVERAGE})
  target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

add_test(unit-test-event-loop ${PROJECT_NAME})
