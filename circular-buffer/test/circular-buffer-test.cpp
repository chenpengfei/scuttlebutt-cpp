//
// Created by 陈鹏飞 on 2020/3/6.
//

#include <thread>
#include <chrono>
#include "gtest/gtest.h"
#include "circular-buffer.h"
#include "scoped-thread/scoped_thread.h"

TEST(buffer_full, circular_buffer) {
    size_t size = 2;
    circular_buffer<std::string> circle(size);

    circle.put("1");
    circle.put("2");
    ASSERT_TRUE(circle.full());

    circle.put("3");
    ASSERT_TRUE(circle.full());
}

TEST(buffer_empty, circular_buffer) {
    size_t size = 2;
    circular_buffer<std::string> circle(size);

    ASSERT_TRUE(circle.empty());

    circle.put("1");
    circle.get();
    ASSERT_TRUE(circle.empty());

    circle.put("3");
    circle.put("4");
    circle.put("5");
    circle.get();
    circle.get();
    ASSERT_TRUE(circle.empty());
}

TEST(thread_safe, circular_buffer) {
    size_t size = 10000;
    circular_buffer<int> circle(size);

    size_t count = 100000;

    auto w_c = 0;
    auto write = [&](){
        while (w_c < count) {
            if (!circle.full()) {
                circle.put(w_c);
                ++w_c;
            }
        }
    };

    auto r_c = 0;
    auto last = -1;
    auto read = [&](){
        while(true) {
            if (!circle.empty()) {
                auto val = circle.get();
                ASSERT_EQ(last + 1, val);
                last = val;
                ++r_c;
                continue;
            }
            if (w_c >= count) {
                return;
            }
        }
    };

    scoped_thread t1{write};
    scoped_thread t2{read};

    ASSERT_EQ(w_c, r_c);
}