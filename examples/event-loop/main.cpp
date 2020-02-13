//
// Created by 陈鹏飞 on 2020/1/19.

#include <iostream>
#include "event_loop.h"
#include "spdlog/spdlog.h"

using namespace std;

EventLoop event_loop("WorkerThread1");


int main() {
    event_loop.start();

    event_loop.once("_update", [](const nonstd::any data) {
        spdlog::info(nonstd::any_cast<string>(data));
    });

    event_loop.emit("_update", string("Hello world"));
    event_loop.emit("_update", string("Good bye"));

    this_thread::sleep_for(1s);

    event_loop.stop();

    return 0;
}


