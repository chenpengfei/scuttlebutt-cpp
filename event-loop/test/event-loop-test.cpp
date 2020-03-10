//
// Created by 陈鹏飞 on 2020/2/11.
//

#include <iostream>
#include "gtest/gtest.h"
#include <event-loop.h>
#include <nonstd/any.h>

using namespace std;
using namespace nonstd;

TEST(event, event_loop) {
    event_loop &el = event_loop::get_instance();

    el.start();

    auto n = 0;
    el.push([&n]() {
        ++n;
    });

    this_thread::sleep_for(100ms);
    ASSERT_EQ(1, n);

    el.stop();
}


