include_directories(include)
include_directories(../circular-buffer/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(SOURCE_FILES circular-buffer-test.cpp)

set(PROJECT_NAME circular-buffer-test)

add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} gtest gtest_main circular-buffer)

if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

add_test(unit-test-circular-buffer ${PROJECT_NAME})
