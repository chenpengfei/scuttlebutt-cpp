//
// Created by 陈鹏飞 on 2020/1/19.
//

#ifndef SCUTTLEBUTT_SINK_H
#define SCUTTLEBUTT_SINK_H

#include <condition_variable>

// https://en.cppreference.com/w/cpp/thread/condition_variable

template<typename T, typename R>
decltype(auto) log(R &&read) {
    std::mutex m;
    std::condition_variable cv;
    std::string data;
    bool ready = false;
    bool ended = false;

    do {
        read(false, [&read, &ended, &ready, &m, &cv](bool done, T val) {
            ended = done;
            if (ended) {
                return;
            }
            std::cout << val << std::endl;

            {
                std::lock_guard<std::mutex> lk(m);
                ready = true;
            }
            cv.notify_one();
        });

        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&ready]{return ready;});
        ready = false;
    } while(!ended);
}

#endif //SCUTTLEBUTT_SINK_H
