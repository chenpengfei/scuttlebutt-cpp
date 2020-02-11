include_directories(include)

set(PULL_INCLUDES
        "include/pull.h"
        "include/log.h"
        "include/values.h"
        "include/map.h"
        )

add_library(pull-stream ${PULL_INCLUDES} "src/dummy.cpp")
