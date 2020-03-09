//
// Created by 陈鹏飞 on 2020/2/19.
//

#include <utility>
#include "scuttlebutt-pull/duplex/include/duplex.h"

namespace sb {

    dp::read &sb::duplex::get_raw_source() {
        if (!raw_source_) {
            decltype(auto) self = this;
            raw_source_ = [self](dp::error abort, dp::source_callback cb) {
                if (dp::end_or_err(abort)) {
                    self->abort_ = abort;
                    // if there is already a cb waiting, abort it.
                    if (self->cb_) {
                        self->callback(abort);
                    }
                }

                if (self->is_first_read_) {
                    self->is_first_read_ = false;
                    auto outgoing = self->get_outgoing();
                    self->push(outgoing, true);
                    self->logger->info("sent 'outgoing': {}", nlohmann::json(outgoing).dump());
                }

                self->cb_ = std::move(cb);
                self->drain();
            };
        }
        return raw_source_;
    }

    dp::sink &duplex::get_raw_sink() {
        if (!raw_sink_) {
            decltype(auto) self = this;
            raw_sink_ = [self](dp::read read) {
                self->peer_read_ = std::move(read);
                self->more_ = [self](dp::error end, const nlohmann::json &u) {
                    if (dp::error::end == end) {
                        self->logger->info("sink ended by peer({}), {}",
                                           self->peer_id_, end);
                        self->end();
                        return;
                    }

                    if (dp::end_or_err(end)) {
                        self->logger->info("sink reading errors, {}", end);
                        self->end();
                        return;
                    }

                    self->logger->info("sink reads data from peer({}): {}",
                            self->peer_id_.empty()? u.get<outgoing>().id_ : self->peer_id_, u.dump());

                    if (u.is_array()) {
                        // counting the update that current stream received
                        ++(self->received_counter_);
                        self->emit("updateReceived", (dp::duplex_pull *) self,
                                   u,
                                   self->received_counter_,
                                   std::string(self->sb_->id_ + "/" +
                                               self->name_));

                        if (!self->writable_) { return; }

                        self->sb_->_update(u.get<sb::update>());
                    } else if (u.is_string()) {
                        auto cmd = u.get<std::string>();
                        if (cmd == "SYNC") {
                            if (self->writable_) {
                                self->logger->info("SYNC received");
                                self->sync_recv_ = true;
                                self->emit("syncReceived");
                                if (self->sync_sent_) {
                                    self->logger->info("emit synced");
                                    self->emit("synced");
                                }
                            } else {
                                self->logger->info(
                                        "ignore peer's({}}) SYNC due to our non-writable setting",
                                        self->peer_id_);
                            }
                        }
                    } else {
                        auto o = u.get<outgoing>();
                        if (self->readable_) {
                            // it's a scuttlebutt digest(vector clocks)
                            self->start(o);
                        } else {
                            self->peer_id_ = o.id_;
                            self->logger->info(
                                    "ignore peer's({}}) outgoing data due to our non-readable setting",
                                    self->peer_id_);
                        }
                    }
                    self->looper_next_();
                };

                self->looper_next_ = self->looper_(std::function<void()>([self]() {
                    self->peer_read_(dp::end_or_err(self->abort_)? self->abort_ : self->ended_, self->more_);
                }));

                self->looper_next_();
            };
        }
        return raw_sink_;
    }

    void duplex::drain() {
        if (!cb_) {
            // there is no downstream waiting for callback
            if (dp::end_or_err(ended_) && on_close_) {
                // perform _onclose regardless of whether there is data in the cache
                auto c = on_close_;
                on_close_ = nullptr;
                c(ended_);
            }
            return;
        }

        if (abort_) {
            // downstream is waiting for abort
            callback(abort_);
        } else if (buffer_.empty() && dp::end_or_err(ended_)) {
            // we'd like to end and there is no left items to be sent
            callback(ended_);
        } else if (!buffer_.empty()) {
            auto payload = buffer_.front();
            buffer_.pop_front();
            callback(dp::error::ok, payload);
        }
    }

    void duplex::start(const outgoing &incoming) {
        logger->info("start with data: {}", nlohmann::json(incoming).dump());

        peer_sources_ = incoming.clock_;
        peer_id_ = incoming.id_;
        peer_accept_ = incoming.accept_;

        std::function<void()> rest = [this, &incoming]() {
            // when we have sent all history
            emit("header", incoming);
            // when we have received all history
            // emit 'synced' when this stream has synced.
            if (sync_recv_) {
                logger->info("emit synced");
                emit("synced");
            }

            if (!tail_) { end(); }
        };

        // won't send history/SYNC abd further update out if the stream is write-only
        if (!readable_) {
            return rest();
        }

        // call this.history to calculate the delta between peers
        std::vector<update> history = sb_->history(peer_sources_, peer_accept_);
        for (auto it = history.begin(); it != history.end(); ++it) {
            std::get<update_items::From>(*it) = sb_->id_;
            push(*it);
        }

        logger->info("'history' to peer({}) has been sent: {}", peer_id_, nlohmann::json(history).dump());

        if (readable_) {
            push(std::string("SYNC"));
            sync_sent_ = true;
            logger->info("'SYNC' has been sent to peer({})", peer_id_);
            emit("syncSent");
        }

        sb_->on("_update", get_on_update());

        rest();
    }
}
