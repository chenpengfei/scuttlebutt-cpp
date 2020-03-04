//
// Created by 陈鹏飞 on 2020/1/19.
//

#ifndef SCUTTLEBUTT_SINK_H
#define SCUTTLEBUTT_SINK_H

#include <condition_variable>
#include <future>
#include <utility>
#include "spdlog/spdlog.h"
#include "pull-looper/include/pull_looper.h"
#include "duplex-pull/include/duplex-pull.h"

namespace pull {

    template<typename T, typename R>
    decltype(auto) log_with_recursive(R &&read) {

        std::function<void(bool, T)> more = [&](bool done, T val) {
            if (!done) {
                spdlog::info(val);
                read(false, more);
            }
        };

        read(false, more);
    }

    template<typename T, typename R>
    decltype(auto) log_with_looper(R &&read) {
        pull::looper looper;
        std::function<void()> next = looper(std::function<void()>([&]() {
            read(dp::error::ok, [&](dp::error done, T val) {
                if (dp::error::ok == done) {
                    spdlog::info(val);
                    next();
                }
            });
        }));

        next();
    }

// https://en.cppreference.com/w/cpp/thread/condition_variable
// https://www.modernescpp.com/index.php/c-core-guidelines-be-aware-of-the-traps-of-condition-variables
    template<typename T, typename R>
    decltype(auto) log_with_condition_variable(R &&read) {
        std::mutex m;
        std::condition_variable cv;
        bool ready = false;
        bool ended = false;

        do {
            read(false, [&read, &ended, &ready, &m, &cv](bool done, T val) {
                ended = done;
                if (!ended) {
                    spdlog::info(val);
                }

                {
                    std::lock_guard<std::mutex> lk(m);
                    ready = true;
                }
                cv.notify_one();
            });

            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [&ready] { return ready; });
            ready = false;
        } while (!ended);
    }

// https://www.modernescpp.com/index.php/thread-synchronization-with-condition-variables-or-tasks
    template<typename T, typename R>
    decltype(auto) log_with_promise(R &&read) {
        bool ended = false;

        do {
            std::promise<void> prom;
            auto fut = prom.get_future();

            read(false, [&read, &ended, &prom](bool done, T val) {
                ended = done;
                if (!ended) {
                    spdlog::info(val);
                }

                prom.set_value();
            });

            fut.wait();
        } while (!ended);
    }
}

#endif //SCUTTLEBUTT_SINK_H
