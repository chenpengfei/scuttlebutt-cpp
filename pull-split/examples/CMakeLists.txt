include_directories(../pull-split/include)

set(PROJECT_NAME pull-split-examples)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} pull-split pull-looper duplex-pull)

if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()


