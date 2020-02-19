//
// Created by 陈鹏飞 on 2020/2/19.
//

#include "timestamp.h"
#include "spdlog/spdlog.h"

int main() {
    auto t1 = timestamp();
    auto t2 = timestamp();
    auto t3 = timestamp();
    auto t4 = timestamp();
    auto t5 = timestamp();

    spdlog::info(t1);
    spdlog::info(t2);
    spdlog::info(t3);
    spdlog::info(t4);
    spdlog::info(t5);

    for (auto i = 0; i < 10; i++) {
        spdlog::info(timestamp());
    }

    return 0;
}

