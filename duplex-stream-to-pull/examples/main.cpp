//
// Created by 陈鹏飞 on 2020/1/19.

#include <iostream>
#include "duplex-stream/include/duplex-stream.h"
#include "duplex-stream-to-pull/include/duplex-stream-to-pull.h"
#include "spdlog/spdlog.h"
#include "duplex-pull/include/duplex-pull.h"

using namespace std;

const size_t MAX_BUFFER_SIZE = 1024;

class my_stream : public duplex_stream {
public:
    my_stream(size_t high_water_mark) : duplex_stream(high_water_mark) {

    }

    bool write(const char *buffer, size_t buffer_len) override {
        auto total_len = w_buffer_len_ + buffer_len;
        if (total_len >= MAX_BUFFER_SIZE) {
            // stream will buffer all written chunks until maximum memory usage occurs,
            // at which point it will abort unconditionally
            return false;
        }

        strncpy(w_buffer_, buffer, buffer_len);
        w_buffer_len_ = total_len;

        return total_len < high_water_mark_;
    }

    size_t read(char *buffer, size_t buffer_len) override {
        auto len = buffer_len <= r_buffer_len_ ? buffer_len : r_buffer_len_;
        strncpy(buffer, r_buffer_, len);
        r_buffer_len_ -= len;
        return len;
    }

    void _destroy() override {
        r_buffer_len_ = 0;
        w_buffer_len_ = 0;
        emit("close");
    }

    void end() override {
        r_buffer_len_ = 0;
        w_buffer_len_ = 0;
        emit("end");
    }

public:
    std::string pop() {
        std::string data(w_buffer_, w_buffer_len_);
        w_buffer_len_ = 0;
        emit("drain");
        return data;
    }

    size_t push(char *buffer, size_t buffer_len) {
        auto total_len = r_buffer_len_ + buffer_len;
        if (total_len >= MAX_BUFFER_SIZE) {
            total_len = MAX_BUFFER_SIZE;
        }
        auto len = total_len - r_buffer_len_;

        strncpy(r_buffer_, buffer, len);
        r_buffer_len_ = total_len;

        emit("readable");

        return len;
    }

private:
    char r_buffer_[MAX_BUFFER_SIZE];
    size_t r_buffer_len_ = 0;

    char w_buffer_[MAX_BUFFER_SIZE];
    size_t w_buffer_len_ = 0;
};

class my_duplex : public dp::duplex_pull {
public:
    void callback(dp::error end, const nonstd::any &data) {
        if (cb_) {
            auto _cb = cb_;
            cb_ = nullptr;
            _cb(end, data);
        }
    }

    void drain() {
        if (abort_) {
            // downstream is waiting for abort
            callback(abort_, nullptr);
        } else if (r_buffer_len_ == 0 && dp::end_or_err(ended_)) {
            // we'd like to end and there is no left items to be sent
            callback(ended_, nullptr);
        } else if (r_buffer_len_ > 0) {
            callback(dp::error::ok,std::string(r_buffer_, r_buffer_len_));
            r_buffer_len_ = 0;
        }
    }

    dp::read &source() override {
        if (!raw_source_) {
            decltype(auto) self = this;
            raw_source_ = [self](dp::error abort, dp::source_callback cb) {
                if (dp::end_or_err(abort)) {
                    self->abort_ = abort;
                    // if there is already a cb waiting, abort it.
                    if (self->cb_) {
                        self->callback(abort, nullptr);
                    }
                }

                self->cb_ = std::move(cb);
                self->drain();
            };
        }
        return raw_source_;
    }

    dp::sink &sink() override {
        if (!raw_sink_) {
            decltype(auto) self = this;
            raw_sink_ = [self](dp::read read) {
                self->peer_read_ = std::move(read);
                self->more_ = [self](dp::error end, const nonstd::any &u) {
                    if (dp::end_or_err(end)) {
                        spdlog::info("sink reading end or error, {}", end);
                        self->end();
                        return;
                    }

                    auto data = nonstd::any_cast<std::string>(u);
                    strncpy(self->w_buffer_, data.c_str(), data.length());
                    self->w_buffer_len_ += data.length();

                    self->looper_next_();
                };

                self->looper_next_ = self->looper_(std::function<void()>([self]() {
                    self->peer_read_(dp::end_or_err(self->abort_) ? self->abort_ : self->ended_,
                                     self->more_);
                }));

                self->looper_next_();
            };
        }
        return raw_sink_;
    }

    void end() override {
        ended_ = dp::error::end;
        r_buffer_len_ = 0;
        w_buffer_len_ = 0;
    }

public:
    std::string pop() {
        std::string data(w_buffer_, w_buffer_len_);
        w_buffer_len_ = 0;
        return data;
    }

    size_t push(char *buffer, size_t buffer_len) {
        auto total_len = r_buffer_len_ + buffer_len;
        if (total_len >= MAX_BUFFER_SIZE) {
            total_len = MAX_BUFFER_SIZE;
        }
        auto len = total_len - r_buffer_len_;

        strncpy(r_buffer_, buffer, len);
        r_buffer_len_ = total_len;

        drain();

        return len;
    }

private:
    dp::error ended_ = dp::error::ok;
    dp::error abort_ = dp::error::ok;

    dp::source_callback cb_;
    dp::read raw_source_ = nullptr;
    dp::sink raw_sink_ = nullptr;

    dp::read peer_read_ = nullptr;
    dp::source_callback more_ = nullptr;
    pull::looper looper_;
    std::function<void()> looper_next_ = nullptr;

private:
    char r_buffer_[MAX_BUFFER_SIZE];
    size_t r_buffer_len_ = 0;

    char w_buffer_[MAX_BUFFER_SIZE];
    size_t w_buffer_len_ = 0;
};

int main() {
    char a_buffer[6] = {'1', '2', '2', '3', '3', '3'};
    char b_buffer[6] = {'a', 'b', 'b', 'c', 'c', 'c'};

    auto cb = [](dp::error err) {
        spdlog::info("end {}", err);
    };
    std::shared_ptr<my_stream> raw_steam = std::make_shared<my_stream>(3);
    std::unique_ptr<dp::duplex_pull> s1 = std::make_unique<duplex_stream_to_pull>( raw_steam, cb);

    auto md = new my_duplex();
    std::unique_ptr<dp::duplex_pull> s2(md);

    dp::link(s1, s2);

    raw_steam->push(a_buffer, 1);
    spdlog::info(md->pop()); // 1
    raw_steam->push(a_buffer + 1, 2);
    spdlog::info(md->pop()); // 22
    raw_steam->push(a_buffer + 3, 3);
    spdlog::info(md->pop()); // 333

    md->push(b_buffer, 1);
    spdlog::info(raw_steam->pop()); // a
    md->push(b_buffer + 1, 2);
    spdlog::info(raw_steam->pop()); // bb
    md->push(b_buffer + 3, 3);
    spdlog::info(raw_steam->pop()); // ccc

    return 0;
}


