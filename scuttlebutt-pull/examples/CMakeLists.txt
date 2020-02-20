include_directories(
        ../scuttlebutt-pull/scuttlebutt/include
        ../scuttlebutt-pull/duplex/include
        ../scuttlebutt-pull/model/include
)

set(PROJECT_NAME scuttlebutt-pull-examples)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} scuttlebutt-pull monotonic-timestamp)
if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

