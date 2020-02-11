//
// Created by 陈鹏飞 on 2020/2/11.
//

#include <iostream>
#include "gtest/gtest.h"
#include <event_loop.h>
#include <nonstd/any.h>

using namespace std;
using namespace nonstd;

TEST(onEvent, event_loop) {
    EventLoop el("event-loop-1");
    el.start();

    el.on("_update", [](any data) {
        ASSERT_EQ("_update_data", any_cast<const char*>(data));
    });

    el.emit("_update", "_update_data");

    el.stop();
}

TEST(onceEvent, event_loop) {
    EventLoop el("event-loop-1");
    el.start();

    el.once("_update", [](any data) {
        ASSERT_EQ("_update_data", any_cast<const char*>(data));
    });

    el.emit("_update", "_update_data");
    el.emit("_update", "_update_data_again");

    el.stop();
}

TEST(listenerCount, event_loop) {
    EventLoop el("event-loop-1");
    el.start();
    el.on("_update", [](any data) {});
    ASSERT_EQ(1, el.listener_count("_update"));
    el.on("_update", [](any data) {});
    ASSERT_EQ(2, el.listener_count("_update"));
    el.once("_update", [](any data) {});
    ASSERT_EQ(3, el.listener_count("_update"));
    el.once("_update", [](any data) {});
    ASSERT_EQ(4, el.listener_count("_update"));

    el.stop();
}

TEST(removeAllListener, event_loop) {
    EventLoop el("event-loop-1");
    el.start();

    el.on("_update", [](any data) {});
    el.on("_update", [](any data) {});
    el.once("_update", [](any data) {});
    el.once("_update", [](any data) {});
    ASSERT_EQ(4, el.listener_count("_update"));

    el.remove_all_listener("_update");
    ASSERT_EQ(0, el.listener_count("_update"));

    el.stop();
}