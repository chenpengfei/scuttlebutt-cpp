include_directories(
        ../scuttlebutt-pull/scuttlebutt/include
        ../scuttlebutt-pull/duplex/include
        ../scuttlebutt-pull/model/include
)

set(SPDLOG_LEVEL_TRACE 0)
set(SPDLOG_LEVEL_DEBUG 1)
set(SPDLOG_LEVEL_INFO 2)
set(SPDLOG_LEVEL_WARN 3)
set(SPDLOG_LEVEL_ERROR 4)
set(SPDLOG_LEVEL_CRITICAL 5)
set(SPDLOG_LEVEL_OFF 6)
add_compile_definitions(SPDLOG_ACTIVE_LEVEL=${SPDLOG_LEVEL_DEBUG})

set(PROJECT_NAME scuttlebutt-pull-examples)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} scuttlebutt-pull)
if (${ENABLE_COVERAGE})
    target_link_libraries(${PROJECT_NAME} -fprofile-arcs -ftest-coverage)
endif ()

