include_directories(../pull-stream/include)

set(PROJECT_NAME pull-stream-examples)

add_executable(${PROJECT_NAME} main.cpp)
if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()


