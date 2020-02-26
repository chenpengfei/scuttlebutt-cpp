include_directories(
        duplex/include
        scuttlebutt/include
        model/include
)

add_library(scuttlebutt-pull
        duplex/src/duplex.cpp
        model/src/model.cpp
        scuttlebutt/src/scuttlebutt.cpp
        scuttlebutt/src/utils.cpp)

target_link_libraries(scuttlebutt-pull pull-looper duplex-pull monotonic-timestamp)
