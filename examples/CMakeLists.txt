#
# Copyright 2016 Google Inc. All Rights Reserved.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
include_directories(include)
include_directories(../pull-stream/pull-stream/include)

set(INCLUDES)

set(SOURCES
  "pull-stream/main.cpp"
)

add_executable(examples-pull-stream pull-stream/main.cpp)
if (${ENABLE_COVERAGE})
    target_link_libraries(examples-pull-stream -fprofile-arcs -ftest-coverage)
endif ()
