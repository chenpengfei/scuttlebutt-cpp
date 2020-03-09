include_directories(../socket-stream/include)

set(PROJECT_NAME socket-stream-examples)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME}
        scuttlebutt-pull
        socket-stream
        duplex-stream-to-pull
        duplex-pull
        pull-looper)

if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()


