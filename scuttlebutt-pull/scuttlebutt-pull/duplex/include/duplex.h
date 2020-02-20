//
// Created by 陈鹏飞 on 2020/2/19.
//

#ifndef SCUTTLEBUTT_DUPLEX_H
#define SCUTTLEBUTT_DUPLEX_H

#include <deque>
#include <typeinfo>
#include <functional>
#include <iostream>
#include <utility>
#include "event-emitter/include/event_emitter.h"
#include "scuttlebutt.h"

namespace sb {

    using on_close = std::function<void(bool)>;

    struct outgoing {
        std::string id_;
        sources clock_;
        nonstd::any meta_;
        model_accept accept_;
    };

    class duplex : public dp::duplex_base {
    public:
        duplex(scuttlebutt *sb, const stream_options &opts)
                : sb_(sb), writable_(opts.writable_), readable_(opts.readable_),
                  tail_(opts.tail_), meta_(opts.meta_) {

            name_ = (!opts.name_.empty() ? opts.name_ : "stream");
            wrapper_ = (!opts.wrapper_.empty() ? opts.wrapper_ : "json");
            logger = sb->logger->ns(name_);

            // Non-writable means we could skip receiving SYNC from peer
            sync_recv_ = !writable_;

            // Non-readable means we don't need to send SYNC to peer
            sync_sent_ = !readable_;

            ++sb->streams;
            once("dispose", get_on_end());

            on_close_ = [this](bool) {
                sb_->remove_listener("_update", get_on_update());
                sb_->remove_listener("dispose", get_on_end());
                --(sb_->streams);
                sb_->emit("unstream", sb_->streams);
            };
        }

        void end(bool end = true) {
            get_on_end()(end);
        }

        dp::read &source() override {
            if (!source_) {
                if (wrapper_ == "raw") {
                    source_ = get_raw_source();
                } else if (wrapper_ == "json") {
                    source_ = get_raw_source();
                }
            }
            return source_;
        }

        dp::sink &sink() override {
            if (!sink_) {
                if (wrapper_ == "raw") {
                    sink_ = get_raw_sink();
                } else if (wrapper_ == "json") {
                    sink_ = get_raw_sink();
                }
            }
            return sink_;
        }

    private:
        std::function<void(bool)> &get_on_end() {
            if (on_end_ == nullptr) {
                on_end_ = [this](bool end) {
                    ended_ = end;
                    // attempt to drain
                    drain();
                };
            }
            return on_end_;
        }

        void drain() {
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
                auto &payload = buffer_.front();
                callback(false, payload);
                buffer_.pop_front();

                // fire this event when the payload has been read by downstream
                if (payload.type() == typeid(update)) {
                    // if payload is an update
                    ++sent_counter_;
                    emit("updateSent", this, payload, sent_counter_, (sb_->id_ + "/" + name_));
                }
            }
        }

        void callback(bool end, const nonstd::any& data = nonstd::any()) {
            auto cb = cb_;
            if (end && on_close_) {
                auto c = on_close_;
                on_close_ = nullptr;
                c(end);
            }
            cb_ = nullptr;
            if (cb) {
                cb(end, data);
            }
        }

        outgoing get_outgoing() {
            return outgoing{sb_->id_, sb_->sources_, meta_, sb_->accept_};
        }

        // process any update occurred on sb
        std::function<void(update &)> &get_on_update() {
            if (on_update_ == nullptr) {
                on_update_ = [this](update &update) {
                    logger->info("got 'update on stream: {}", "update");//todo

                    // current stream is in write-only mode
                    if (!readable_) {
                        logger->info("'update' ignored by it's non-readable flag");
                        return;
                    }

                    if (!_filter(update, peer_sources_)) { return; }

                    // this update comes from our peer stream, don't send back
                    if (std::get<update_items::From>(update) == peer_id_) {
                        logger->info("'update' ignored by peerId: {}", peer_id_);
                        return;
                    }

                    bool is_accepted = sb_->is_accepted(peer_accept_, update);

                    if (!is_accepted) {
                        logger->info("'update ignored by peerAccept: {} {}", "update", "peer_accept_");//todo
                        return;
                    }

                    // send 'scuttlebutt' to peer
                    std::get<update_items::From>(update) = sb_->id_;
                    push(update);
                    logger->info("sent 'update' to peer: {}", "update");//todo

                    // really, this should happen before emitting.
                    auto ts = std::get<update_items::Timestamp>(update);
                    auto source = std::get<update_items::SourceId>(update);
                    peer_sources_[source] = ts;
                    logger->info("updated peerSources to", "peer_sources_");//todo
                };
            }
            return on_update_;
        }

        dp::read &get_raw_source() {
            if (!raw_source_) {
                raw_source_ = [this](bool abort, dp::source_callback cb){
                    if (abort) {
                        abort_ = abort;
                        // if there is already a cb waiting, abort it.
                        if (cb_) {
                            callback(abort);
                        }
                    }

                    if (is_first_read_) {
                        is_first_read_ = false;
                        auto outgoing = get_outgoing();
                        push(outgoing, true);
                        logger->info("sent 'outgoing': {} [{} {}]",
                                outgoing.id_, outgoing.clock_.begin()->first,
                                outgoing.clock_.begin()->second);//todo.outgoing
                    }

                    cb_ = std::move(cb);
                    drain();
                };
            }
            return raw_source_;
        }

        dp::sink &get_raw_sink() {
            if (!raw_sink_) {
                raw_sink_ = [this](dp::read read){
                    bool _ended = false;

                    do {
                        std::promise<void> prom;
                        auto fut= prom.get_future();

                        read(abort_ || ended_, [this, &read, &_ended, &prom](bool end, const nonstd::any &u) {
                            _ended = end;

                            if (end) {
                                logger->info("sink ended by peer({}), {}", peer_id_, end);
                                this->end(end);
                                return;
                            }

                            logger->info("sink reads data from peer({}): {}",
                                    !peer_id_.empty()? peer_id_: nonstd::any_cast<outgoing>(u).id_, "update");//todo

                            if (u.type() == typeid(sb::update)) {
                                // counting the update that current stream received
                                ++received_counter_;
                                emit("updateReceived", this, u, received_counter_, (sb_->id_ + "/" + name_));

                                if (!writable_) { return; }

                                sb_->_update(nonstd::any_cast<sb::update>(u));
                            } else if (u.type() == typeid(std::string)) {
                                auto cmd = nonstd::any_cast<std::string>(u);
                                if (cmd == "SYNC") {
                                    if (writable_) {
                                        logger->info("SYNC received");
                                        sync_recv_ = true;
                                        emit("syncReceived");
                                        if (sync_sent_) {
                                            logger->info("emit synced");
                                            emit("synced");
                                        }
                                    } else {
                                        logger->info("ignore peer's({}}) SYNC due to our non-writable setting", peer_id_);
                                    }
                                }
                            } else {
                                outgoing o = nonstd::any_cast<outgoing>(u);
                                logger->info("sink reads data from peer({}): {} [{} {}]",
                                             peer_id_, o.id_, o.clock_.begin()->first,o.clock_.begin()->second);//todo

                                if (readable_) {
                                    // it's a scuttlebutt digest(vector clocks)
                                    start(o);
                                } else {
                                    peer_id_ = o.id_;
                                    logger->info("ignore peer's({}}) outgoing data due to our non-readable setting", peer_id_);
                                }
                            }

                            prom.set_value();
                        });

                        fut.wait();
                    } while(!_ended);

                };
            }
            return raw_sink_;
        }

        void push(nonstd::any data, bool to_head = false) {
            if (ended_) { return; }

            // if sink already waiting,
            // we can call back directly.
            if (cb_) {
                callback(abort_, data);
                return;
            }

            // otherwise buffer data
            if (to_head) {
                buffer_.push_front(data);
            } else {
                buffer_.push_back(data);
            }
        }

        void start(const outgoing &incoming) {
            logger->info("start with data: {} [{} {}]",
                    incoming.id_, incoming.clock_.begin()->first,incoming.clock_.begin()->second);//todo

            auto &peer_sources = incoming.clock_;
            auto &peer_id = incoming.id_;
            auto &peer_accept = incoming.accept_;

            std::function<void()> rest = [this, &incoming](){
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
            std::vector<update> history = sb_->history(this->peer_sources_, this->peer_accept_);
            for (auto it = history.begin(); it != history.end(); ++it) {
                std::get<update_items::From>(*it) = sb_->id_;
                push(*it);
            }

            logger->info("'history' to peer({}) has been sent: {}", peer_id, "history");//todo
            sb_->on("_update", get_on_update());

            if (readable_) {
                push("SYNC");
                sync_sent_ = true;
                logger->info("'SYNC' has been sent to peer({})", peer_id);
                emit("syncSent");
            }

            rest();
        }

        std::string name() {
            return name_;
        }

        bool readable() {
            return readable_;
        }

        void readable(bool value) {
            readable_ = value;
        }

        bool writable() {
            return writable_;
        }

        void writable(bool value) {
            writable_ = value;
        }

    private:
        std::function<void(bool end)> on_end_ = nullptr;
        std::function<void(update & update)> on_update_ = nullptr;
        dp::read raw_source_ = nullptr;
        dp::sink raw_sink_ = nullptr;

    private:
        scuttlebutt *sb_ = nullptr;
        std::string name_ = "stream";
        dp::read source_ = nullptr;
        dp::sink sink_ = nullptr;
        std::string wrapper_ = "json";
        bool readable_ = true;
        bool writable_ = true;
        bool ended_ = false;
        bool abort_ = false;
        bool sync_sent_ = false;
        bool sync_recv_ = false;
        std::deque<nonstd::any> buffer_;
        dp::source_callback cb_;
        on_close on_close_ = nullptr;
        bool is_first_read_ = true;
        int sent_counter_ = 0; // update count that the stream has sent
        int received_counter_ = 0; // update count that the stream has received
        bool tail_ = true;
        nonstd::any meta_;
        std::shared_ptr<debug> logger;

        sources peer_sources_;
        model_accept peer_accept_;
        source_id peer_id_ = "";
    };
}

#endif //SCUTTLEBUTT_DUPLEX_H
