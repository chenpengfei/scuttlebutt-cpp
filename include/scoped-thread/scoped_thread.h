//
// Created by 陈鹏飞 on 2020/3/6.
//

#ifndef SCUTTLEBUTT_SCOPED_THREAD_H
#define SCUTTLEBUTT_SCOPED_THREAD_H

#include <thread>

class scoped_thread {
public:
    template<typename... Arg>
    scoped_thread(Arg &&... arg)
            : thread_(std::forward<Arg>(arg)...) {}

    scoped_thread(scoped_thread &&other)
            : thread_(std::move(other.thread_)) {}

    scoped_thread(const scoped_thread &) = delete;

    ~scoped_thread() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

private:
    std::thread thread_;
};

#endif //SCUTTLEBUTT_SCOPED_THREAD_H
