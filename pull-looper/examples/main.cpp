//
// Created by 陈鹏飞 on 2020/1/19.

#include "spdlog/spdlog.h"
#include "pull_looper.h"

using namespace std;

int main() {

    float N = 1000000;
    float n = N, c = 0;
    const clock_t begin_time = clock();

    std::function<void()> next = pull::looper(std::function<void()>([&]() {
        c++;
        if (--n > 0) { return next(); }
        auto ms = float(clock() - begin_time) / (CLOCKS_PER_SEC / 1000);
        spdlog::info("time for 1m loop, {}ms", ms);
        spdlog::info("loops per ms, {}", N / ms);
    }));

    next();

    return 0;
}

