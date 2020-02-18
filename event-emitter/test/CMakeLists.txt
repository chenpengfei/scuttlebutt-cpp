include_directories(../event-emitter/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(SOURCE_FILES event-emitter-test.cpp)

set(PROJECT_NAME event-emitter-test)

add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} gtest gtest_main event-emitter)

if (${ENABLE_COVERAGE})
  target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

add_test(unit-test-event-emitter ${PROJECT_NAME})
