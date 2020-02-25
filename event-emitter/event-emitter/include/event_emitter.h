//
// Created by 陈鹏飞 on 2020/2/17.
//

#ifndef SCUTTLEBUTT_EVENT_EMITTER_H
#define SCUTTLEBUTT_EVENT_EMITTER_H

#include <iostream>
#include <map>
#include <vector>
#include "nonstd/any.h"

struct any_callable {
    any_callable() {}

    template<typename ... Args>
    any_callable(const std::function<void(Args...)> &callable, bool once)
            : callable_(callable), once_(once), address_(size_t(&callable)) {
    }

    template<typename ... Args>
    void operator()(Args &&... args) {
        (nonstd::any_cast<std::function<void(Args...)>>(callable_))(std::forward<Args>(args)...);
    }

    bool operator==(const any_callable &cb) {
        return ((this->once_ == cb.once_) && (this->address_ == cb.address_));
    }

    nonstd::any callable_;
    bool once_;
    size_t address_;
};

class event_emitter {
public:
    virtual ~event_emitter() {}

    /**
     * Synchronously calls each of the listeners registered for the event named eventName,
     * in the order they were registered, passing the supplied arguments to each.
     * @param[in] event_name - The name of the event
     * @param[in] args - the argument passed to the listeners registered for the event name
     * @return Returns true if the event had listeners, false otherwise.
     */
    template<class... Args>
    bool emit(const std::string &event_name, Args &&... args) {
        auto copy = listeners(event_name);

        if (copy.empty()) {
            return false;
        }

        for (auto it = copy.begin(); it != copy.end(); ++it) {
            if (it->once_) {
                auto success = remove_listener(event_name, *it);
                assert(success);
            }
            it->operator()(std::forward<Args>(args)...);
        }

        return true;
    }

    /**
     * Adds the listener function to the end of the listeners array for the event named eventName.
     * No checks are made to see if the listener has already been added.
     * Multiple calls passing the same combination of eventName and listener
     * will result in the listener being added, and called, multiple times.
     * @param event_name - The name of the event
     * @param listener - The callback function
     * @return Returns a reference to the EventEmitter, so that calls can be chained.
     */
    template<typename ... Args>
    event_emitter &on(const std::string &event_name, const std::function<void(Args...)> &listener) {
        add_listener(event_name, listener, false);
        return *this;
    }

    /**
     * Adds a one-time listener function for the event named eventName.
     * The next time eventName is triggered, this listener is removed and then invoked.
     */
    template<typename ... Args>
    event_emitter &
    once(const std::string &event_name, const std::function<void(Args...)> &listener) {
        add_listener(event_name, listener, true);
        return *this;
    }

    /**
     * Removes the specified listener from the listener array for the event named event_name.
     * remove_listener() will remove, at most, one instance of a listener from the listener array.
     * If any single listener has been added multiple times to the listener array for the specified eventName,
     * then remove_listener() must be called multiple times to remove each instance.
     *
     * When a single function has been added as a handler multiple times for a single event (as in the example below),
     * remove_listener() will remove the most recently added instance
     */
    template<typename ... Args>
    event_emitter &
    remove_listener(const std::string &event_name, const std::function<void(Args...)> &listener) {
        auto cbs_it = listeners_.find(event_name);
        if (cbs_it != listeners_.end()) {
            for (auto ri = cbs_it->second.rbegin(); ri != cbs_it->second.rend(); ++ri) {
                if (ri->address_ == (size_t) (&listener)) {
                    cbs_it->second.erase(std::next(ri).base());
                    break;
                }
            }
        }
        return *this;
    }

    /**
     * Removes all listeners, or those of the specified eventName.
     */
    event_emitter &remove_all_listener(const std::string &event_name) {
        listeners_.erase(event_name);
        return *this;
    }

    /**
     * Returns the number of listeners listening to the event named eventName.
     * @param event_name - The name of the event being listened for
     * @return
     */
    size_t listener_count(const std::string &event_name) {
        auto it = listeners_.find(event_name);
        if (it == listeners_.end()) {
            return 0;
        } else {
            return it->second.size();
        }
    }

    /**
     * Returns a copy of the array of listeners for the event named event_name
     * @param event_name
     * @return
     */
    std::vector<any_callable> listeners(const std::string &event_name) {
        auto cbs_it = listeners_.find(event_name);

        if (cbs_it == listeners_.end()) {
            return std::vector<any_callable>();
        }

        return cbs_it->second;
    }

private:
    template<typename ... Args>
    void add_listener(const std::string &event_name, const std::function<void(Args...)> &listener,
                      bool once = false) {
        decltype(auto) it = listeners_.find(event_name);
        if (it == listeners_.end()) {
            std::vector<any_callable> vec;
            vec.emplace_back(listener, once);
            listeners_[event_name] = vec;
        } else {
            it->second.emplace_back(listener, once);
        }
    }

    /**
     * Removes the specified listener from the listener array for the event named event_name.
     * remove_listener() will remove, at most, one instance of a listener from the listener array.
     */
    bool remove_listener(const std::string &event_name, const any_callable &cb) {
        auto cbs_it = listeners_.find(event_name);
        if (cbs_it != listeners_.end()) {
            for (auto it = cbs_it->second.begin(); it != cbs_it->second.end(); ++it) {
                if (*it == cb) {
                    cbs_it->second.erase(it);
                    return true;
                }
            }
        }
        return false;
    }


private:
    std::map<std::string, std::vector<any_callable>> listeners_;
};

#endif //SCUTTLEBUTT_EVENT_EMITTER_H
