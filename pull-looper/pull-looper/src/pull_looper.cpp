//
// Created by 陈鹏飞 on 2020/2/26.
//

#include "pull_looper.h"

namespace pull {

    std::function<void()> looper::operator()(std::function<void()> f) {
        return [this, f]() {
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