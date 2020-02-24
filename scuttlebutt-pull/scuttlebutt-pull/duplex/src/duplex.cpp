//
// Created by 陈鹏飞 on 2020/2/19.
//

#include <utility>
#include <scuttlebutt-pull/duplex/include/duplex.h>

namespace sb {

    dp::read &sb::duplex::get_raw_source() {
        if (!raw_source_) {
            decltype(auto) self = this;
            raw_source_ = [self](bool abort, dp::source_callback cb) {
                if (abort) {
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
                    self->logger->info("sent 'outgoing': {} [{} {}]",
                                       outgoing.id_, outgoing.clock_.begin()->first,
                                       outgoing.clock_.begin()->second);//todo.outgoing
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
                self->peer_next_ = [self](bool end, const nonstd::any &u) {
                    if (end) {
                        self->logger->info("sink ended by peer({}), {}", self->peer_id_, end);
                        self->end(end);
                        return;
                    }

                    if (u.type() == typeid(sb::update)) {
                        self->logger->info("sink reads data from peer({}): {}", self->peer_id_,
                                           "update");//todo

                        // counting the update that current stream received
                        ++(self->received_counter_);
                        self->emit("updateReceived", (dp::duplex_base *) self, u,
                                   self->received_counter_,
                                   std::string(self->sb_->id_ + "/" + self->name_));

                        if (!self->writable_) { return; }

                        self->sb_->_update(nonstd::any_cast<sb::update>(u));
                    } else if (u.type() == typeid(std::string)) {
                        self->logger->info("sink reads data from peer({}): {}",
                                           self->peer_id_, nonstd::any_cast<std::string>(u));//todo

                        auto cmd = nonstd::any_cast<std::string>(u);
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
                        outgoing o = nonstd::any_cast<outgoing>(u);
                        self->logger->info("sink reads data from peer({}): {} [{} {}]",
                                           self->peer_id_, o.id_, o.clock_.begin()->first,
                                           o.clock_.begin()->second);//todo

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

                    self->peer_read_(self->abort_ || self->ended_, self->peer_next_);
                };

                self->peer_read_(self->abort_ || self->ended_, self->peer_next_);
            };
        }
        return raw_sink_;
    }

//    template<typename T, typename R>
//    decltype(auto) log_with_recursive(R &&read) {
//
//        std::function<void(bool, T)> more = [&](bool done, T val) {
//            if (!done) {
//                spdlog::info(val);
//                read(false, more);
//            }
//        };
//
//        read(false, more);
//    }

//    dp::sink &duplex::get_raw_sink() {
//        if (!raw_sink_) {
//            raw_sink_ = [this](dp::read read){
//                bool _ended = false;
//
//                do {
//                    std::promise<void> prom;
//                    auto fut= prom.get_future();
//
//                    read(abort_ || ended_, [this, &read, &_ended, &prom](bool end, const nonstd::any &u) {
//                        _ended = end;
//
//                        if (end) {
//                            logger->info("sink ended by peer({}), {}", peer_id_, end);
//                            this->end(end);
//                            return;
//                        }
//
//                        logger->info("sink reads data from peer({}): {}",
//                                     !peer_id_.empty()? peer_id_: nonstd::any_cast<outgoing>(u).id_, "update");//todo
//
//                        if (u.type() == typeid(sb::update)) {
//                            // counting the update that current stream received
//                            ++received_counter_;
//                            emit("updateReceived", this, u, received_counter_, (sb_->id_ + "/" + name_));
//
//                            if (!writable_) { return; }
//
//                            sb_->_update(nonstd::any_cast<sb::update>(u));
//                        } else if (u.type() == typeid(std::string)) {
//                            auto cmd = nonstd::any_cast<std::string>(u);
//                            if (cmd == "SYNC") {
//                                if (writable_) {
//                                    logger->info("SYNC received");
//                                    sync_recv_ = true;
//                                    emit("syncReceived");
//                                    if (sync_sent_) {
//                                        logger->info("emit synced");
//                                        emit("synced");
//                                    }
//                                } else {
//                                    logger->info("ignore peer's({}}) SYNC due to our non-writable setting", peer_id_);
//                                }
//                            }
//                        } else {
//                            outgoing o = nonstd::any_cast<outgoing>(u);
//                            logger->info("sink reads data from peer({}): {} [{} {}]",
//                                         peer_id_, o.id_, o.clock_.begin()->first,o.clock_.begin()->second);//todo
//
//                            if (readable_) {
//                                // it's a scuttlebutt digest(vector clocks)
//                                start(o);
//                            } else {
//                                peer_id_ = o.id_;
//                                logger->info("ignore peer's({}}) outgoing data due to our non-readable setting", peer_id_);
//                            }
//                        }
//
//                        prom.set_value();
//                    });
//
//                    fut.wait();
//                } while(!_ended);
//
//            };
//        }
//        return raw_sink_;
//    }

    void duplex::drain() {
        if (!cb_) {
            // there is no downstream waiting for callback
            if (ended_ && on_close_) {
                // perform _onclose regardless of whether there is data in the cache
                auto c(on_close_);
                on_close_ = nullptr;
                c(ended_);
            }
            return;
        }

        if (abort_) {
            // downstream is waiting for abort
            callback(abort_);
        } else if (buffer_.empty() && ended_) {
            // we'd like to end and there is no left items to be sent
            callback(ended_);
        } else if (!buffer_.empty()) {
            auto payload = buffer_.front();
            buffer_.pop_front();
            callback(false, payload);
        }
    }

    void duplex::start(const outgoing &incoming) {
        logger->info("start with data: {} [{} {}]",
                     incoming.id_, incoming.clock_.begin()->first,
                     incoming.clock_.begin()->second);//todo

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

        logger->info("'history' to peer({}) has been sent: {}", peer_id_, "history");//todo
        sb_->on("_update", get_on_update());

        if (readable_) {
            push(std::string("SYNC"));
            sync_sent_ = true;
            logger->info("'SYNC' has been sent to peer({})", peer_id_);
            emit("syncSent");
        }

        rest();
    }
}
