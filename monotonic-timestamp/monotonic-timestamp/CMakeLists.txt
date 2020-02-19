include_directories(include)

set(SOURCE_FILES src/timestamp.cpp)

add_library(monotonic-timestamp ${SOURCE_FILES})
