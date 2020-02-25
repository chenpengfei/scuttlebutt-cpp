//
// Created by 陈鹏飞 on 2020/2/16.
//

#include <stdlib.h>
#include "scuttlebutt.h"
#include "debug/debug.h"
#include "duplex.h"
#include "monotonic-timestamp/include/timestamp.h"

const auto default_logger = debug::create("sb");

namespace sb {

    scuttlebutt::scuttlebutt(scuttlebutt_options opts)
            : accept_(opts.accept_) {
        auto id = opts.id_;

        if (opts.sign_ && opts.verify_) {
            set_id(id.empty() ? (opts.create_id_ ? opts.create_id_() : id) : id);
        } else {
            set_id(id.empty() ? _create_id() : id);
        }

        logger = default_logger->ns(id_);
        accept_ = opts.accept_;
    }

    bool scuttlebutt::_update(sb::update update) {
        auto data_any = std::get<update_items::Data>(update);
        auto data = nonstd::any_cast<std::pair<std::string, nonstd::any>>(data_any);
        auto key = std::get<model_value_items::Key>(data);
        auto value = std::get<model_value_items::Value>(data);
        logger->info("_update: [ [{} {}] {} {} ]",
                     key,
                     nonstd::any_cast<std::string>(value),
                     std::get<sb::update_items::Timestamp>(update),
                     std::get<sb::update_items::SourceId>(update));//todo

        auto ts = std::get<update_items::Timestamp>(update);
        auto source_id = std::get<update_items::SourceId>(update);
        auto it = sources_.find(source_id);
        auto latest = (it != sources_.end() ? it->second : 0);

        if (latest > ts) {
            logger->info("update is older, ignore it {} {} {}", latest, ts, abs(latest - ts));
            emit("old_data", update);
            return false;
        }

        sources_[source_id] = ts;
        logger->info("update our sources to {} {}", source_id, ts);//todo.sources_

        auto did_verification = [this, &update](bool verified) {
            // I'm not sure how what should happen if a async verification
            // errors. if it's an key not found - that is a verification fail,
            // not a error. if it's genuine error, really you should queue and
            // try again? or replay the message later
            // -- this should be done my the security plugin though, not scuttlebutt.
            if (!verified) {
                this->emit("unverified_data", update);
                return false;
            }

            // emit '_update' event to notify every streams on this SB
            bool r = this->apply_updates(update);
            if (r) {
                this->emit("_update", update);
                this->logger->info("applied 'update' and fired ⚡_update, total listeners: {}",
                                   this->listener_count("_update"));
            }

            return r;
        };

        if (source_id != id_) {
            return verify_ ? did_verification(verify_(update)) : did_verification(true);
        } else {
            if (sign_) {
                std::get<update_items::Singed>(update) = sign_(update);
            }
            return did_verification(true);
        }
    }

    bool scuttlebutt::local_update(const std::pair<std::string, nonstd::any> trx) {
        return _update(std::make_tuple(trx, monotonic_timestamp::timestamp(), id_, "", ""));
    }

    dp::duplex_base *scuttlebutt::create_stream(const stream_options &opts = stream_options{}) {
        return new duplex(this, opts);
    }

    dp::duplex_base *scuttlebutt::create_write_stream(stream_options &opts) {
        opts.writable_ = true;
        opts.readable_ = false;
        return create_stream(opts);
    }

    dp::duplex_base *scuttlebutt::create_sink_stream(stream_options &opts) {
        opts.writable_ = true;
        opts.readable_ = false;
        return create_stream(opts);
    }

    dp::duplex_base *scuttlebutt::create_read_stream(stream_options &opts) {
        opts.writable_ = false;
        opts.readable_ = true;
        return create_stream(opts);
    }

    dp::duplex_base *scuttlebutt::create_source_stream(stream_options &opts) {
        opts.writable_ = false;
        opts.readable_ = true;
        return create_stream(opts);
    }
}
