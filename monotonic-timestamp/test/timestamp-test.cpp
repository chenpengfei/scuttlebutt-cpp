//
// Created by 陈鹏飞 on 2020/2/19.
//

#include "gtest/gtest.h"
#include "timestamp.h"

// assert that timestamps are strictly increasing!
TEST(timestamp, monotonic_timestamp) {
    auto max = 1000;
    auto l = max;
    auto prev = 0;

    while(l--) {
        auto t = timestamp();
        if (prev >= t) {
            ASSERT_TRUE(prev < t);
        }
        prev = t;
    }
}

