include_directories(
        duplex/include
        scuttlebutt/include
        model/include
)

add_library(scuttlebutt-pull
        duplex/src/duplex.cpp
        duplex/src/json.cpp
        model/src/model.cpp
        scuttlebutt/src/scuttlebutt.cpp
        scuttlebutt/src/json.cpp
        scuttlebutt/src/utils.cpp)

target_link_libraries(scuttlebutt-pull
        event-loop
        pull-looper
        duplex-pull
        monotonic-timestamp)
