//
// Created by 陈鹏飞 on 2020/2/12.
//

#pragma once

#ifndef SCUTTLEBUTT_DEBUG_H
#define SCUTTLEBUTT_DEBUG_H

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/base_sink.h"

template<typename Mutex>
class my_sink : public spdlog::sinks::base_sink<Mutex> {
protected:
    void sink_it_(const spdlog::details::log_msg &msg) override {
        if (msg.logger_name == last_logger_name_) {
            spdlog::sinks::base_sink<Mutex>::set_pattern_(pattern_without_logger_name_);
        } else {
            last_logger_name_ = msg.logger_name;
            spdlog::sinks::base_sink<Mutex>::set_pattern_(pattern_);
        }

        // log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
        // msg.raw contains pre formatted log

        // If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        std::cout << fmt::to_string(formatted);
    }

    void flush_() override {
        std::cout << std::flush;
    }

public:
    static spdlog::string_view_t last_logger_name_;

private:
    const std::string pattern_ = "[%^%L%$][%t] %-10n %v";
    const std::string pattern_without_logger_name_ = "[%^%L%$][%t]            %v";
};

template<class Mutex>
spdlog::string_view_t my_sink<Mutex>::last_logger_name_ = "";

#include "spdlog/details/null_mutex.h"
#include <mutex>

using my_sink_mt = my_sink<std::mutex>;
using my_sink_st = my_sink<spdlog::details::null_mutex>;

class debug : public spdlog::logger {
public:
    debug() : spdlog::logger("") {}

    debug(const std::string &logger_name) : spdlog::logger(logger_name,
                                                           std::make_shared<my_sink_mt>()) {
    }

    static auto create(const std::string &logger_name) {
#ifdef NDEBUG
        return std::make_shared<debug>();
#else
        return std::make_shared<debug>(logger_name);
#endif /* NDEBUG */
    }

    auto ns(const std::string &logger_name) {
#ifdef NDEBUG
        return std::make_shared<debug>();
#else
        return std::make_shared<debug>(name_ + ":" + logger_name);
#endif /* NDEBUG */
    }
};

#endif //SCUTTLEBUTT_DEBUG_H
