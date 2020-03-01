include_directories(../duplex-stream-to-pull/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(SOURCE_FILES duplex-stream-to-pull-test.cpp)

set(PROJECT_NAME duplex-stream-to-pull-test)

add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
        gmock gmock_main gtest gtest_main
        duplex-stream-to-pull pull-stream duplex-pull pull-looper)

if (${ENABLE_COVERAGE})
  target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

add_test(unit-test-duplex-stream-to-pull ${PROJECT_NAME})
