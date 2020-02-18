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
