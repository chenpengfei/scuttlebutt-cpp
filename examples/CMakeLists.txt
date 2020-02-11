include_directories(
        ../pull-stream/pull-stream/include
        ../event-loop/event-loop/include
)

add_executable(examples-pull-stream pull-stream/main.cpp)
if (${ENABLE_COVERAGE})
    target_link_libraries(examples-pull-stream -fprofile-arcs -ftest-coverage)
endif ()

add_executable(examples-event-loop event-loop/main.cpp)
target_link_libraries(examples-event-loop event-loop)
if (${ENABLE_COVERAGE})
    target_link_libraries(examples-event-loop -fprofile-arcs -ftest-coverage)
endif ()

