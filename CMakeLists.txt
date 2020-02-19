cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 14)

SET(GTEST_LATEST false CACHE BOOL "Download latest gtest from github or local gtest zip")
SET(ENABLE_COVERAGE false CACHE BOOL "Enable test coverage with GCC or clang")

if (${GTEST_LATEST})
    # Download and unpack googletest at configure time
    configure_file(CMakeLists.txt.in
            googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

    # Prevent GoogleTest from overriding our compiler/linker options
    # when building with Visual Studio
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This adds
    # the following targets: gtest, gtest_main, gmock
    # and gmock_main
    add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
            ${CMAKE_BINARY_DIR}/googletest-build)

    enable_testing()
else()
    set(GTEST googletest-release-1.8.1)
    include_directories("./include" "${GTEST}/googletest/include/")
    link_directories("build/gtest/googlemock/gtest/")
endif ()

if (${ENABLE_COVERAGE})
    add_compile_options(-g -O0 -fprofile-arcs -ftest-coverage)
endif ()

project(scuttlebutt-cpp)

include_directories(
        include
        event-emitter/event-emitter/include)

add_subdirectory(pull-stream)
add_subdirectory(event-emitter)
add_subdirectory(event-loop)
add_subdirectory(monotonic-timestamp)
add_subdirectory(scuttlebutt-pull)


