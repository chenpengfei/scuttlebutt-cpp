//
// Created by 陈鹏飞 on 2020/3/4.
//

#ifndef SCUTTLEBUTT_PULL_THROUGH_H
#define SCUTTLEBUTT_PULL_THROUGH_H

#include <iostream>
#include <queue>
#include "nlohmann/json.hpp"
#include "duplex-pull/include/duplex-pull.h"
#include "pull-looper/include/pull_looper.h"

namespace pull {

    using writer = std::function<void(const std::string &)>;
    using ender = std::function<void()>;

    class pull_through final {
    public:
        virtual ~pull_through() = default;

        pull_through(pull::writer writer = nullptr, pull::ender ender = nullptr)
                : writer_(std::move(writer)), ender_(std::move(ender)) {
            auto self = this;
            if (writer_ == nullptr) {
                writer_ = [self](const std::string &data) {
                    self->queue_.push(data);
                };
            }
            if (ender_ == nullptr) {
                ender_ = [self]() {
                    self->queue_.push("");
                };
            }
        }

        dp::read operator()(dp::read read) {
            peer_read_ = std::move(read);

            auto self = this;
            return [self](dp::error end, dp::source_callback cb) {
                self->ended_ = dp::end_or_err(self->ended_) ? self->ended_ : end;

                if (dp::end_or_err(end)) {
                    return self->peer_read_(end,
                            [self, end, cb](dp::error, const nlohmann::json &) {
                                if (self->cb_) {
                                    auto t = self->cb_;
                                    self->cb_ = nullptr;
                                    t(end, nullptr);
                                }
                                cb(end, nullptr);
                            });
                }

                self->cb_ = std::move(cb);

                self->looper_ = pull::looper();
                self->looper_next_ = self->looper_(std::function<void()>([self]() {
                    //if it's an error
                    if (!self->cb_) return;

                    if (dp::end_or_err(self->error_)) {
                        auto cb = self->cb_;
                        self->cb_ = nullptr;
                        cb(self->error_, nullptr);
                    } else if (!self->queue_.empty()) {
                        auto data = self->queue_.front();
                        self->queue_.pop();
                        auto cb = self->cb_;
                        self->cb_ = nullptr;
                        //todo
                        cb(data.empty() ? dp::error::end : dp::error::ok,
                           data.empty() ? nlohmann::json("") : nlohmann::json::parse(data));
                    } else {
                        self->peer_read_(self->ended_, self->more_);
                    }
                }));

                self->more_ = [self](dp::error end, const nlohmann::json &data) {
                    // null has no special meaning for pull-stream
                    if (dp::end_or_err(end) && end != dp::error::end) {
                        self->error_ = end;
                        return self->looper_next_();//todo
                    }

                    self->ended_ = dp::end_or_err(self->ended_) ? self->ended_ : end;
                    if (dp::end_or_err(self->ended_)) {
                        self->ender_();
                    } else if (!data.empty()) {
                        self->writer_(data);
                        if (dp::end_or_err(self->error_) || dp::end_or_err(self->ended_)) {
                            auto err = dp::end_or_err(self->error_) ? self->error_ : self->ended_;
                            return self->peer_read_(err,
                                        [self, err](dp::error, const nlohmann::json &) {
                                            auto cb = self->cb_;
                                            self->cb_ = nullptr;
                                            cb(err, nullptr);
                                        });
                        }
                    }

                    self->looper_next_();
                };

                self->looper_next_();
            };
        }

        void enqueue(const std::string &data) {
            queue_.push(data);
        }

        void emit(const std::string &event, const std::string &data) {
            if (event == "data") enqueue(data);
            if (event == "end") {
                ended_ = dp::error::end;
                enqueue(nullptr);
            }
            if (event == "error") error_ = dp::error::err;
        }

    private:
        pull::writer writer_ = nullptr;
        pull::ender ender_ = nullptr;

        std::queue<std::string> queue_;
        dp::error ended_ = dp::error::ok;
        dp::error error_ = dp::error::ok;

        dp::read peer_read_ = nullptr;
        dp::source_callback cb_ = nullptr;

        pull::looper looper_;
        std::function<void()> looper_next_ = nullptr;
        dp::source_callback more_ = nullptr;
    };
}

#endif //SCUTTLEBUTT_PULL_THROUGH_H
