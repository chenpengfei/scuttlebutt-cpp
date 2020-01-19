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
include_directories(../pull-stream/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(INCLUDES)

set(SOURCES
  "src/pull-test.cpp"
  "src/map-test.cpp"
  "src/values-test.cpp"
)

add_executable(pull-stream-test ${INCLUDES} ${SOURCES})
target_link_libraries(pull-stream-test gtest gtest_main)
if (${ENABLE_COVERAGE})
  target_link_libraries(pull-stream-test -fprofile-arcs -ftest-coverage)
endif ()

add_test(PullStreamUnitTests pull-stream-test)
