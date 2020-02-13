//
// Created by 陈鹏飞 on 2020/1/19.

#include <deque>
#include <vector>
#include <iterator>
#include <pull.h>
#include <log.h>
#include <values.h>
#include <map.h>
#include "spdlog/spdlog.h"

using namespace std;

void test_pull_stream_with_promise() {
    int n = 5;
    vector<int> vec;
    vec.reserve(n);
    for (auto i = 1; i < n; ++i) {
        vec.push_back(i);
    }

    auto begin = vec.begin();
    auto end = vec.end();
    auto vals = values(begin, end);
    auto mapper = [&](int val){return val * 2;};
    auto timesTwo = Map<int>(mapper);
    auto timesFour = pipe_through(timesTwo, timesTwo);
    auto logInt = [&] (auto read) { log_with_promise<int>(read); };

    auto newVals = pull(vals, timesTwo, timesFour);

    const clock_t begin_time = clock();

    pull(vals, timesTwo, logInt);

    spdlog::info("test_pull_stream_with_promise cost: {}ms",
            float( clock () - begin_time ) /  (CLOCKS_PER_SEC/1000));

}

void test_pull_stream_with_condition_variable() {
    int n = 5;
    vector<int> vec;
    vec.reserve(n);
    for (auto i = 1; i < n; ++i) {
        vec.push_back(i);
    }

    auto begin = vec.begin();
    auto end = vec.end();
    auto vals = values(begin, end);
    auto mapper = [&](int val){return val * 2;};
    auto timesTwo = Map<int>(mapper);
    auto timesFour = pipe_through(timesTwo, timesTwo);
    auto logInt = [&] (auto read) { log_with_condition_variable<int>(read); };

    auto newVals = pull(vals, timesTwo, timesFour);
    auto newLogInt = pipe_through(timesTwo, timesTwo, timesFour, logInt);

    const clock_t begin_time = clock();

    pull(newVals,
            timesTwo,
            pipe_through(timesTwo, timesTwo),
            timesFour,
            timesTwo,
            newLogInt);

    spdlog::info("test_pull_stream_with_condition_variable cost: {}ms",
                 float( clock () - begin_time ) /  (CLOCKS_PER_SEC/1000));
}

int main() {
    test_pull_stream_with_promise();

    test_pull_stream_with_condition_variable();

    return 0;
}

