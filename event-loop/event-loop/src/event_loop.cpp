#include <assert.h>

#include <utility>
#include "event_loop.h"
#include "spdlog/spdlog.h"

using namespace std;
using namespace nonstd;

const char *EventLoop::EXIT_THREAD_EVENT_NAME = "__exit_thread_event_name__";

EventLoop::EventLoop(const char *threadName) : work_thread_exit_(false), work_thread_(nullptr),
                                               THREAD_NAME_(threadName) {
}

EventLoop::~EventLoop() {
    stop();
}

bool EventLoop::start() {
    if (!work_thread_) {
        work_thread_exit_ = false;
        work_thread_ = new thread(&EventLoop::work_thread, this);

        once(EXIT_THREAD_EVENT_NAME, [=](const nonstd::any &) {
            // clear event queue
            std::unique_lock<std::mutex> lk(this->mutex_);
            while (!this->queue_.empty()) {
                auto event = this->queue_.front();
                this->queue_.pop();
                delete event;
            }

            // exit work thread
            this->work_thread_exit_ = true;
        });
    }

    return true;
}

std::thread::id EventLoop::get_thread_id() {
    assert(work_thread_ != nullptr);
    return work_thread_->get_id();
}

std::thread::id EventLoop::get_current_thread_id() {
    return this_thread::get_id();
}

void EventLoop::stop() {
    if (!work_thread_)
        return;

    push(new Event(EXIT_THREAD_EVENT_NAME, nullptr));

    work_thread_->join();
    delete work_thread_;
    work_thread_ = nullptr;
}

void EventLoop::work_thread() {
    while (!work_thread_exit_) {
        Event *event = nullptr;
        {
            // Wait for a event to be added to the queue
            std::unique_lock<std::mutex> lk(mutex_);
            while (queue_.empty())
                cv_.wait(lk);

            if (queue_.empty())
                continue;

            event = queue_.front();
            queue_.pop();
        }

        auto map_it = listeners_.find(event->name_);
        if (map_it != listeners_.end()) {
            auto &eh = map_it->second;
            auto it = eh.begin();
            while (it != eh.end()) {
                if (it->once_) {
                    auto _h = it->handler_;
                    it = eh.erase(it);
                    _h(event->arg_);
                } else {
                    it->handler_(event->arg_);
                    ++it;
                }
            }
        }

        delete event;
    }

    spdlog::info("Exit work thread on {}",  THREAD_NAME_);
}

bool EventLoop::emit(const std::string& eventName, any arg) {
    assert(work_thread_);

    push(new Event(eventName, std::move(arg)));
    return true;
}

EventLoop &EventLoop::on(const string& eventName, const Handler &listener) {
    add_listener(eventName, listener, false);
    return *this;
}

EventLoop &EventLoop::once(const string& eventName, const Handler &listener) {
    add_listener(eventName, listener, true);
    return *this;
}

EventLoop &EventLoop::remove_all_listener(const std::string& eventName) {
    listeners_.erase(eventName);
    return *this;
}

int EventLoop::listener_count(const std::string& eventName) {
    auto it = listeners_.find(eventName);
    if (it == listeners_.end()) {
        return 0;
    } else {
        return it->second.size();
    }
}

void EventLoop::add_listener(const std::string& eventName, const Handler &listener, bool once) {
    auto it = listeners_.find(eventName);
    if (it == listeners_.end()) {
        std::vector<EventHandler> vec;
        vec.emplace_back(listener, once);
        listeners_[eventName] = vec;
    } else {
        it->second.emplace_back(listener, once);
    }
}

void EventLoop::push(Event *event) {
    std::unique_lock<std::mutex> lk(mutex_);
    queue_.push(event);
    cv_.notify_one();
}

