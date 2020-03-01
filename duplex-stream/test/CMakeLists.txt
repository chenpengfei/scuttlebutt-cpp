include_directories(../duplex-stream/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(SOURCE_FILES duplex-stream-test.cpp)

set(PROJECT_NAME duplex-stream-test)

add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} gtest gtest_main duplex-stream)

if (${ENABLE_COVERAGE})
  target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

add_test(unit-test-duplex-stream ${PROJECT_NAME})
