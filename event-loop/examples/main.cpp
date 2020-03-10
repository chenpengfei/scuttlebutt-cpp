//
// Created by 陈鹏飞 on 2020/1/19.

#include <iostream>
#include "event-loop.h"
#include "spdlog/spdlog.h"

using namespace std;

int main() {
    event_loop &el = event_loop::get_instance();

    el.start();

    auto msg = "world 1";
    el.push([msg]() {
      spdlog::info("hello {}", msg);
    });

    msg = "world 2";
    el.push([msg]() {
        spdlog::info("hello {}", msg);
    });

    this_thread::sleep_for(1s);

    el.stop();

    return 0;
}


