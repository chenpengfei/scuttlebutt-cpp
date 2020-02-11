#ifndef _THREAD_STD_H
#define _THREAD_STD_H

#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <map>
#include <condition_variable>
#include <nonstd/any.h>

using Handler = std::function<void(nonstd::any)>;

struct EventHandler {
    EventHandler(const Handler &handler, bool once) : handler_(handler), once_(once) {
    }

    Handler handler_;
    bool once_;
};

struct Event {
    Event(std::string name, nonstd::any arg) : name_(name),
                                               arg_(arg) {
    }

    std::string name_;
    nonstd::any arg_;
};

class EventLoop {
public:
    EventLoop(const EventLoop &) = delete;

    EventLoop &operator=(const EventLoop &) = delete;

    EventLoop(const char *threadName);

    ~EventLoop();

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
     * Get the ID of the currently executing thread
     * @return The current thread ID
     */
    static std::thread::id get_current_thread_id();

    /**
     * Asynchronously calls each of the listeners registered for the event named eventName,
     * in the order they were registered, passing the supplied arguments to each.
     *
     * @param[in] eventName - The name of the event
     * @param[in] arg - the argument passed to the listeners registered for the event name
     * @return Returns true if the event had listeners, false otherwise.
     */
    bool emit(std::string eventName, nonstd::any arg);

    /**
     * Adds the listener function to the end of the listeners array for the event named eventName.
     * No checks are made to see if the listener has already been added.
     * Multiple calls passing the same combination of eventName and listener
     * will result in the listener being added, and called, multiple times.
     *
     * @param eventName - The name of the event
     * @param listener - The callback function
     * @return Returns a reference to the EventEmitter, so that calls can be chained.
     */
    EventLoop &on(std::string eventName, const Handler &listener);

    /**
     * Adds a one-time listener function for the event named eventName.
     * The next time eventName is triggered, this listener is removed and then invoked.
     *
     */
    EventLoop &once(std::string eventName, const Handler &listener);

    /**
     * Removes all listeners, or those of the specified eventName.
     *
     */
    EventLoop &remove_all_listener(std::string eventName);

    /**
     * Returns the number of listeners listening to the event named eventName.
     *
     * @param eventName - The name of the event being listened for
     * @return
     */
    int listener_count(std::string eventName);

private:
    // Entry point for the worker thread
    void work_thread();

    // Add msg to queue and notify worker thread
    void push(Event *event);

    // Add listener for the event named eventName
    void add_listener(std::string eventName, const Handler &listener, bool once = false);

    const char *THREAD_NAME_;
    std::atomic<bool> work_thread_exit_;
    std::thread *work_thread_;
    std::map<std::string, std::vector<EventHandler>> listeners_;
    std::queue<Event *> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;

    static const char *EXIT_THREAD_EVENT_NAME;
};

#endif 

