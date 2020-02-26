include_directories(../pull-looper/include)

set(PROJECT_NAME pull-looper-examples)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} pull-looper)

if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()


