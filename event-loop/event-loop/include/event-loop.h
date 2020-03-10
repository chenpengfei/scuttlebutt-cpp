//
// Created by 陈鹏飞 on 2020/2/19.
//

#ifndef SCUTTLEBUTT_EVENT_LOOP_H
#define SCUTTLEBUTT_EVENT_LOOP_H

#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <map>
#include <condition_variable>
#include <utility>
#include <nonstd/any.h>

namespace el {

    using handler = std::function<void()>;

    class event_loop final {
    public:
        event_loop(const event_loop &) = delete;

        event_loop &operator=(const event_loop &) = delete;

        static event_loop& get_instance() {
            static event_loop    instance; // Guaranteed to be destroyed.
            // Instantiated on first use.
            return instance;
        }

        /**
         * Called once to create the worker thread
         * @return TRUE if thread is created. FALSE otherwise.
         */
        bool start();

        /**
         * Called once a program exit to exit the worker thread
         */
        void stop();

        /**
         * Get the ID of this thread instance
         * @return The worker thread ID
         */
        std::thread::id get_thread_id();

        /**
         * Add msg to queue and notify worker thread
         */
        void push(const handler& handler);

    private:
        event_loop() : work_thread_exit_(false) {}

        // Entry point for the worker thread
        void work_thread();

        const char *THREAD_NAME_ = "event-loop";
        std::atomic<bool> work_thread_exit_;
        std::thread *work_thread_ = nullptr;

        std::queue<handler> queue_;
        std::mutex mutex_;
        std::condition_variable cv_;
    };

    #define START() event_loop::get_instance().start()
    #define STOP() event_loop::get_instance().stop()
    #define PUSH(f) event_loop::get_instance().push(f)
}

#endif //SCUTTLEBUTT_EVENT_LOOP_H
