include_directories(../duplex-stream-to-pull-to-pull/include)

set(PROJECT_NAME duplex-stream-to-pull-examples)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} duplex-stream-to-pull duplex-pull pull-looper)
if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

