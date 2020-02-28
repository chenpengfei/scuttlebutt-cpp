//
// Created by 陈鹏飞 on 2020/1/19.

#include <deque>
#include <iterator>
#include <pull.h>
#include <log.h>
#include <values.h>
#include <map.h>
#include "spdlog/spdlog.h"

using namespace std;
using namespace pull;

void test_pull() {
    int n = 5;
    vector<int> vec;
    vec.reserve(n);
    for (auto i = 1; i < n; ++i) {
        vec.push_back(i);
    }

    auto begin = vec.begin();
    auto end = vec.end();
    auto vals = pull::values(begin, end);
    auto mapper = [&](int val){return val * 2;};
    auto timesTwo = pull::Map<int>(mapper);
    auto timesFour = pipe_through(timesTwo, timesTwo);
    auto logInt = [&] (auto read) { pull::log_with_looper<int>(read); };

    auto newVals = pull::pull(vals, timesTwo, timesFour);
    auto newLogInt = pull::pipe_through(timesTwo, timesTwo, timesFour, logInt);

    const clock_t begin_time = clock();

    pull::pull(newVals,
         timesTwo,
         pipe_through(timesTwo, timesTwo),
         timesFour,
         timesTwo,
         newLogInt);

    spdlog::info("test_pull_stream_with_condition_variable cost: {}ms",
                 float( clock () - begin_time ) /  (CLOCKS_PER_SEC/1000));
}

int main() {
    int n = 500000;
    auto source = pull::values(n);

    auto log_int = [&](auto read) { pull::log_with_looper<int>(read); };
//    auto log_int = [&](auto read) { pull::log_with_promise<int>(read); };

    const clock_t begin_time = clock();
    log_int(source);
    auto ms = float(clock() - begin_time) / (CLOCKS_PER_SEC / 1000);
    spdlog::info("time for 1m loop, {}ms", ms);


    return 0;
}

