include_directories(../event-emitter/include)

set(PROJECT_NAME event-emitter-examples)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} event-emitter)
if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

