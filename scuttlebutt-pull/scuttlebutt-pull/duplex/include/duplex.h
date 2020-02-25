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

    class duplex final : public dp::duplex_base {
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
            sb_->once("dispose", get_on_end());

            on_close_ = std::function<void(bool)>([this](bool) {
                sb_->remove_listener("_update", get_on_update());
                sb_->remove_listener("dispose", get_on_end());
                --(sb_->streams);
                sb_->emit("unstream", sb_->streams);
            });
        }

        void end() override {
            get_on_end()();
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
        std::function<void()> &get_on_end() {
            if (on_end_ == nullptr) {
                on_end_ = [this]() {
                    ended_ = true;
                    // attempt to drain
                    drain();
                };
            }
            return on_end_;
        }

        void drain();

        void callback(bool end, const nonstd::any &data = nonstd::any()) {
            auto cb = cb_;
            if (end && on_close_) {
                auto c = on_close_;
                on_close_ = nullptr;
                c(end);
            }
            cb_ = nullptr;
            if (cb) {
                cb(end, data);

                // fire this event when the payload has been read by downstream
                if (data.type() == typeid(sb::update)) {
                    // if payload is an update
                    ++sent_counter_;
                    emit("updateSent", (dp::duplex_base *) this, data, sent_counter_,
                         std::string(sb_->id_ + "/" + name_));
                }
            }
        }

        outgoing get_outgoing() {
            return outgoing{sb_->id_, sb_->sources_, meta_, sb_->accept_};
        }

        // process any update occurred on sb
        std::function<void(sb::update &)> &get_on_update() {
            if (on_update_ == nullptr) {
                on_update_ = [this](sb::update update) {
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
                        logger->info("'update ignored by peerAccept: {} {}", "update",
                                     "peer_accept_");//todo
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

        dp::read &get_raw_source();

        dp::sink &get_raw_sink();

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

        void start(const outgoing &incoming);

    public:
        std::string name() override {
            return name_;
        }

        bool readable() override {
            return readable_;
        }

        void readable(bool value) override {
            readable_ = value;
        }

        bool writable() override {
            return writable_;
        }

        void writable(bool value) override {
            writable_ = value;
        }

    private:
        std::function<void()> on_end_ = nullptr;
        std::function<void(sb::update &update)> on_update_ = nullptr;
        dp::read raw_source_ = nullptr;
        dp::sink raw_sink_ = nullptr;

        dp::read peer_read_ = nullptr;
        dp::source_callback peer_next_ = nullptr;

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
