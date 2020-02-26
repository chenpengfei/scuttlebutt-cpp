//
// Created by 陈鹏飞 on 2020/2/26.
//

#include "gtest/gtest.h"
#include "pull_looper.h"
#include "spdlog/spdlog.h"

TEST(without_stackoverflow, pull_looper) {
    float N = 1000000;
    float n = N, c = 0;
    const clock_t begin_time = clock();

    pull::looper looper;
    std::function<void()> next = looper(std::function<void()>([&]() {
        c++;
        if (--n > 0) { return next(); }
        auto ms = float(clock() - begin_time) / (CLOCKS_PER_SEC / 1000);
        spdlog::info("time for 1m loop, {}ms", ms);
        spdlog::info("loops per ms, {}", N / ms);
    }));

    next();
}

TEST(compare_to_recursive, pull_looper) {
    float N = 10000;
    float n = N, c = 0;
    const clock_t begin_time = clock();

    std::function<void()> next = [&]() {
        c++;
        if (--n > 0) { return next(); }
        auto ms = float(clock() - begin_time) / (CLOCKS_PER_SEC / 1000);
        spdlog::info("time for 1m loop, {}ms", 100 * ms);
        spdlog::info("loops per ms, {}", N / ms);
    };

    next();
}