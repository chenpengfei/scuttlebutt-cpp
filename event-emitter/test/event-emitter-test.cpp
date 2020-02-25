//
// Created by 陈鹏飞 on 2020/2/11.
//

#include <iostream>
#include "gtest/gtest.h"
#include <nonstd/any.h>
#include "event_emitter.h"

using namespace std;
using namespace nonstd;

struct foo : public event_emitter {
public:
    foo() {}

    std::function<void(int)> &get_adder() {
        if (add_ == nullptr) {
            add_ = [=](int a) {
                this->sum_ += a;
            };
        }
        return add_;
    }


    std::function<void(int)> add_ = nullptr;
    int sum_ = 0;
};

TEST(on, event_emitter) {
    string name("_adder");

    foo foo_1;
    foo_1.on(name, foo_1.get_adder());
    foo_1.emit(name, 1);
    ASSERT_EQ(1, foo_1.sum_);

    foo foo_2;
    foo_2.on(name, foo_2.get_adder());
    foo_2.emit(name, 2);
    ASSERT_EQ(2, foo_2.sum_);
}

TEST(once, event_emitter) {
    string name("_adder");

    foo foo_1;
    foo_1.once(name, foo_1.get_adder());
    foo_1.emit(name, 1);
    foo_1.emit(name, 2);
    ASSERT_EQ(1, foo_1.sum_);

    foo foo_2;
    foo_2.once(name, foo_2.get_adder());
    ASSERT_EQ(1, foo_2.listener_count(name));
    foo_2.emit(name, 3);
    ASSERT_EQ(3, foo_2.sum_);
    ASSERT_EQ(0, foo_2.listener_count(name));
}

TEST(remove_listener, event_emitter) {
    foo foo_1;
    foo foo_2;
    string name("_adder");

    foo_1.on(name, foo_1.get_adder());
    foo_2.remove_listener(name, foo_2.get_adder());
    ASSERT_EQ(1, foo_1.listener_count(name));

    foo_1.remove_listener(name, foo_1.get_adder());
    ASSERT_EQ(0, foo_1.listener_count(name));
}

TEST(remove_all_listener, event_emitter) {
    string name("_adder");

    foo foo_1;
    foo foo_2;
    foo_1.on(name, foo_1.get_adder());
    foo_1.on(name, foo_1.get_adder());
    foo_2.on(name, foo_2.get_adder());
    foo_2.on(name, foo_2.get_adder());
    ASSERT_EQ(2, foo_1.listener_count(name));
    ASSERT_EQ(2, foo_2.listener_count(name));

    foo_1.remove_all_listener(name);
    foo_2.remove_all_listener(name);
    ASSERT_EQ(0, foo_1.listener_count(name));
    ASSERT_EQ(0, foo_2.listener_count(name));
}

TEST(listener_count, event_emitter) {
    string name("_adder");

    foo foo_1;
    foo foo_2;
    foo_1.on(name, foo_1.get_adder());
    foo_1.on(name, foo_1.get_adder());
    foo_2.on(name, foo_2.get_adder());
    foo_2.on(name, foo_2.get_adder());
    ASSERT_EQ(2, foo_1.listener_count(name));
    ASSERT_EQ(2, foo_2.listener_count(name));

    foo_1.remove_listener(name, foo_1.get_adder());
    foo_2.remove_listener(name, foo_2.get_adder());
    ASSERT_EQ(1, foo_1.listener_count(name));
    ASSERT_EQ(1, foo_2.listener_count(name));
}

TEST(remove_the_most_recently_added_instance, event_emitter) {
    auto ee = event_emitter();

    auto pong = std::function<void()>([]() {
        std::cout << "pong" << std::endl;
    });

    ee.on("ping", pong);
    ee.once("ping", pong);
    ee.remove_listener("ping", pong);

    ASSERT_EQ(1, ee.listener_count("ping"));

    auto copy = ee.listeners("ping");
    ASSERT_EQ(1, copy.size());
    for (auto it = copy.begin(); it != copy.end(); ++it) {
        ASSERT_EQ(false, it->once_);
    }
}

TEST(remove_listener_in_emit_progress, event_emitter) {
    auto ee = event_emitter();

    int counter_b = 0;
    int counter_a = 0;
    auto cb_b = std::function<void()>([&]() {
        ++counter_b;
    });

    auto cb_a = std::function<void()>([&]() {
        ++counter_a;
        ee.remove_listener("event", cb_b);
    });

    ee.on("event", cb_a);
    ee.on("event", cb_b);

    ee.emit("event");
    ASSERT_EQ(1, counter_a);
    ASSERT_EQ(1, counter_b);

    ee.emit("event");
    ASSERT_EQ(2, counter_a);
    ASSERT_EQ(1, counter_b);
}

