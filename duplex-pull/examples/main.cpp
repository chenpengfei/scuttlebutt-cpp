//
// Created by 陈鹏飞 on 2020/1/19.

#include <iostream>
#include "event-emitter/include/event_emitter.h"
#include "spdlog/spdlog.h"

using namespace std;

class foo : public event_emitter {
public:
    foo() {
        on("_adder"s, get_adder());
        once("_print"s, get_printer());
    }

    int add(int a) {
        emit("_adder"s, int(a));
        return sum_;
    }

    std::function<void(int)> &get_adder() {
        if (add_ == nullptr) {
            add_ = [=](int a) {
                this->sum_ += a;
            };
        }
        return add_;
    }

    std::function<void(string &)> &get_printer() {
        if (print_ == nullptr) {
            print_ = [=](string &str) {
                spdlog::info(str);
            };
        }
        return print_;
    }

    std::function<void(int)> add_ = nullptr;
    std::function<void(string &)> print_ = nullptr;
    int sum_ = 0;
};

int main() {
    foo foo_;
    spdlog::info(foo_.add(3));
    spdlog::info(foo_.add(4));

    string str("hello");
    foo_.emit("_print"s, str);
    foo_.emit("_print"s, str);

    foo foo_1;
    foo foo_2;
    foo_1.on("_adder_"s, foo_1.get_adder());
    foo_2.remove_listener("_adder_"s, foo_2.get_adder());
    spdlog::info(foo_1.listener_count("_adder_"s));

    return 0;
}


