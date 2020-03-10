//
// Created by 陈鹏飞 on 2020/3/2.
//

#ifndef SCUTTLEBUTT_FAKE_DUPLEX_PULL_H
#define SCUTTLEBUTT_FAKE_DUPLEX_PULL_H

#include <queue>
#include "nlohmann/json.hpp"
#include "pull-split/include/pull-split.h"
#include "pull-through/include/pull-through.h"
#include "pull-stream/include/pull.h"
#include "event-loop/include/event-loop.h"

const size_t MAX_BUFFER_SIZE = 100 * 1024;

class fake_duplex_pull : public dp::duplex_pull {
public:
    void callback(dp::error end, const nlohmann::json &data) {
        if (cb_) {
            auto _cb = cb_;
            cb_ = nullptr;
            _cb(end, data);
        }
    }

    void drain() {
        if (!cb_) {
            return;
        }

        if (abort_) {
            // downstream is waiting for abort
            callback(abort_, nullptr);
        } else if (buffer_.empty() && dp::end_or_err(ended_)) {
            // we'd like to end and there is no left items to be sent
            callback(ended_, nullptr);
        } else if (!buffer_.empty()) {
            std::string data = buffer_.front();
            buffer_.pop();
            //todo
            //spdlog::info("fake stream callback data {}", data);
            callback(dp::error::ok, data);
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

    dp::sink &get_raw_sink() {
        if (!raw_sink_) {
            decltype(auto) self = this;
            raw_sink_ = [self](dp::read read) {
                self->peer_read_ = std::move(read);
                self->more_ = [self](dp::error end, const nlohmann::json &u) {
                    if (dp::end_or_err(end)) {
                        spdlog::info("sink reading end or error, {}", end);
                        self->end();
                        return;
                    }

                    auto data = u.get<int>();
                    self->emit("data", data);

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

    dp::sink &sink() override {
        if (!sink_) {
            if (wrapper_ == "raw") {
                sink_ = get_raw_sink();
            } else if (wrapper_ == "json") {
                sink_ = pull::pipe_through(json_parser_, get_raw_sink());
            }
        }
        return sink_;
    }

    void end() override {
        ended_ = dp::error::end;
    }

public:
    void push(const std::string& data) {
        el::PUSH(el::handler([this, data](){
            buffer_.push(data);
            //todo
            //spdlog::info("push data to fake stream: {}", data);
            drain();
        }));
    }

private:
    dp::error ended_ = dp::error::ok;
    dp::error abort_ = dp::error::ok;

    std::string wrapper_ = "json";

    dp::source_callback cb_;
    dp::read raw_source_ = nullptr;
    dp::sink raw_sink_ = nullptr;
    dp::sink sink_ = nullptr;

    dp::read peer_read_ = nullptr;
    dp::source_callback more_ = nullptr;
    pull::looper looper_;
    std::function<void()> looper_next_ = nullptr;

    pull::pull_split json_parser_;

private:
    std::queue<std::string> buffer_;
};

#endif //SCUTTLEBUTT_FAKE_DUPLEX_PULL_H
