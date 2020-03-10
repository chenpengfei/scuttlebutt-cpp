#include <assert.h>
#include "event-loop.h"
#include "spdlog/spdlog.h"

namespace el {

    bool event_loop::start() {
        if (!work_thread_) {
            work_thread_exit_ = false;
            work_thread_ = new std::thread(&event_loop::work_thread, this);
        }
        return true;
    }

    std::thread::id event_loop::get_thread_id() {
        assert(work_thread_ != nullptr);
        return work_thread_->get_id();
    }

    void event_loop::stop() {
        if (!work_thread_)
            return;

        push([=]() {
            // clear event queue
            std::lock_guard<std::mutex> lk(this->mutex_);
            while (!this->queue_.empty()) {
                auto event = this->queue_.front();
                this->queue_.pop();
            }

            // exit work thread
            this->work_thread_exit_ = true;
        });

        work_thread_->join();
        delete work_thread_;
        work_thread_ = nullptr;
    }

    void event_loop::work_thread() {
        while (!work_thread_exit_) {
            handler _handler;
            {
                // Wait for a event to be added to the queue
                std::unique_lock<std::mutex> lk(mutex_);
                while (queue_.empty())
                    cv_.wait(lk);

                if (queue_.empty())
                    continue;

                _handler = queue_.front();
                queue_.pop();
            }
            _handler();
        }

        spdlog::info("Exit work thread on {}", THREAD_NAME_);
    }

    void event_loop::push(const handler& handler) {
        assert(work_thread_);

        std::lock_guard<std::mutex> lk(mutex_);
        queue_.push(handler);
        cv_.notify_one();
    }
}

