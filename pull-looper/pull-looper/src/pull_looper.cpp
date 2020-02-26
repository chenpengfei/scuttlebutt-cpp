//
// Created by 陈鹏飞 on 2020/2/26.
//

#include "pull_looper.h"

namespace pull {

    std::function<void()> looper(std::function<void()> f) {
        bool active = false, called = false;
        return [&]() {
            called = true;
            if (!active) {
                active = true;
                while (called) {
                    called = false;
                    f();
                }
                active = false;
            }
        };
    }
}