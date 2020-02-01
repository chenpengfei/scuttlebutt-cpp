//
// Created by 陈鹏飞 on 2020/1/19.

#include <deque>
#include <vector>
#include <iterator>
#include <functional>
#include <iostream>
#include <cstdarg>
#include <pull.h>
#include <log.h>
#include <values.h>
#include <map.h>

using namespace std;

int main() {
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
    auto logInt = [&] (auto read) { log<int>(read); };

    pull(pull(vals, timesTwo, timesFour, timesTwo, timesFour),
            timesTwo,
            pipe_through(timesTwo, timesTwo),
            timesFour,
            timesTwo,
            pipe_through(timesTwo, timesFour, timesTwo, timesFour, logInt));

    puts("over");

    return 0;
}

