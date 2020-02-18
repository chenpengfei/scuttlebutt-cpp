include_directories(../event-loop/include)

set(PROJECT_NAME event-loop-examples)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} event-loop)
if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()
