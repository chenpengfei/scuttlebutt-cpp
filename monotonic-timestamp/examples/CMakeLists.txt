include_directories(../monotonic-timestamp/include)

set(PROJECT_NAME monotonic-timestamp-examples)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} monotonic-timestamp)
if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()
