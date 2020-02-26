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
#include "pull-looper/include/pull_looper.h"

using namespace std;

int main() {
    int n = 500000;
    auto source = pull::values(n);

    auto log_int = [&](auto read) { log_with_looper<int>(read); };
//    auto log_int = [&](auto read) { pull::log_with_promise<int>(read); };

    const clock_t begin_time = clock();
    log_int(source);
    auto ms = float(clock() - begin_time) / (CLOCKS_PER_SEC / 1000);
    spdlog::info("time for 1m loop, {}ms", ms);


    return 0;
}

