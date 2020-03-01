//
// Created by 陈鹏飞 on 2020/2/29.
//

#include "duplex-stream-to-pull.h"

duplex_stream_to_pull::duplex_stream_to_pull(std::shared_ptr<duplex_stream> stream,
                                             std::function<void(dp::error)> cb)
        : stream_(std::move(stream)), w_end_cb_(std::move(cb)) {
    r_read_ = [this]() {
        auto size = stream_->read(r_buffer_, MAX_READ_SIZE);
        if (size != 0 && r_cb_ != nullptr) {
            auto cb = r_cb_;
            r_cb_ = nullptr;
            std::string data(r_buffer_, size);
            cb(dp::error::ok, data);
        }
    };

    r_on_readable_ = [this]() {
        r_waiting_ = true;
        if (r_cb_) {
            r_read_();
        }
    };

    r_on_end_ = [this]() {
        r_ended_ = dp::error::end;
        if (r_cb_) {
            r_cb_(r_ended_, nullptr);
        }
    };

    r_on_error_ = [this](dp::error err) {
        r_ended_ = err;
        if (r_cb_) {
            r_cb_(r_ended_, nullptr);
        }
    };

    stream_->on("readable", r_on_readable_);
    stream_->on("end", r_on_end_);
    stream_->on("error", r_on_error_);
}

dp::read &duplex_stream_to_pull::source() {
    if (!raw_source_) {
        decltype(auto) self = this;

        raw_source_ = [self](dp::error abort, dp::source_callback cb) {
            self->r_cb_ = cb;
            if (dp::end_or_err(self->r_ended_)) {
                cb(self->r_ended_, nullptr);
            } else if (self->r_waiting_) {
                self->r_read_();
            }
        };
    }
    return raw_source_;
}

dp::sink &duplex_stream_to_pull::sink() {
    if (!raw_sink_) {
        decltype(auto) self = this;

        w_done_ = [self](dp::error, const nonstd::any &) {
            if (self->w_did_) { return; }
            self->w_did_ = true;
            if (self->w_end_cb_ != nullptr) {
                self->w_end_cb_(self->w_ended_ ? dp::error::ok : self->w_ended_);
            }
        };

        w_on_close_ = [self]() {
            if (self->w_closed_) { return; }
            self->w_closed_ = true;
            self->w_cleanup_();
            if (!dp::end_or_err(self->w_ended_)) {
                self->peer_read_(self->w_ended_ = dp::error::end, self->w_done_);
            } else {
                self->w_done_(dp::error::end, nullptr);
            }
        };

        w_on_error_ = [self](dp::error err) {
            self->w_cleanup_();
            if (!dp::end_or_err(self->w_ended_)) {
                self->peer_read_(self->w_ended_ = err, self->w_done_);
            }
        };

        w_cleanup_ = [self]() {
            self->stream_->remove_listener("finish", self->w_on_close_);
            self->stream_->remove_listener("close", self->w_on_close_);
            self->stream_->remove_listener("error", self->w_on_error_);
        };

        self->stream_->on("finish", self->w_on_close_);
        self->stream_->on("close", self->w_on_close_);
        self->stream_->on("error", self->w_on_error_);

        raw_sink_ = [self](dp::read read) {
            self->peer_read_ = std::move(read);
            self->more_ = [self](dp::error end, const nonstd::any &u) {
                self->w_ended_ = dp::end_or_err(self->w_ended_) ? self->w_ended_ : end;

                if (self->w_ended_ == dp::error::end) {
                    return self->stream_->end();
                }

                if (dp::end_or_err(self->w_ended_)) {
                    self->stream_->destroy();
                    return self->w_done_(self->w_ended_, nullptr);
                }

                auto data = nonstd::any_cast<std::string>(u);
                bool pause = self->stream_->write(data.c_str(), data.length());
                if (!pause) {
                    self->stream_->once("drain", self->looper_next_);
                } else {
                    self->looper_next_();
                }
            };

            self->looper_next_ = self->looper_(std::function<void()>([self]() {
                self->peer_read_(dp::error::ok, self->more_);
            }));

            self->looper_next_();
        };
    }
    return raw_sink_;
}

void duplex_stream_to_pull::end() {
    stream_->end();
}


