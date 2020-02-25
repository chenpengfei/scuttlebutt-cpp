//
// Created by 陈鹏飞 on 2020/2/16.
//

#ifndef SCUTTLEBUTT_SCUTTLEBUTT_H
#define SCUTTLEBUTT_SCUTTLEBUTT_H

#include <iostream>
#include <vector>
#include <map>
#include <async_model_store.h>
#include "nonstd/any.h"
#include "debug/debug.h"
#include "event-emitter/include/event_emitter.h"
#include "duplex-pull/include/duplex-pull.h"
#include "util.h"

namespace sb {

    using source_id = std::string;
    using timestamp = double;
    using from = std::string;
    using singed = std::string;

    using sources = std::map<source_id, timestamp>;

    using create_id = std::function<source_id()>;

    enum model_value_items {
        Key = 0,
        Value
    };

    struct model_accept {
        std::vector<std::string> whitelist_;
        std::vector<std::string> blacklist_;
    };

    struct scuttlebutt_options {
        scuttlebutt_options(std::string id = "") : id_(id) {}

        scuttlebutt_options(std::string id, model_accept accept)
                : id_(id), accept_(accept) {}

        source_id id_ = "";
        create_id create_id_ = nullptr;
        bool sign_ = false;
        bool verify_ = false;
        std::unique_ptr<async_model_store_base> store_ = nullptr;
        model_accept accept_;
    };

    struct stream_options {
        stream_options(std::string name = "") : name_(name) {}

        bool readable_ = true;
        bool writable_ = true;
        bool tail_ = true;
        std::string name_ = "";
        sources clock_;
        bool send_clock_ = true;
        std::string wrapper_ = "";
        nonstd::any meta_;
    };

    enum update_items {
        Data = 0,
        Timestamp,
        SourceId,
        From,
        Singed,
    };

    using update = std::tuple<nonstd::any, timestamp, source_id, from, singed>;
    using verify = std::function<bool(const update &)>;
    using sign = std::function<singed(const update &)>;

    std::string _create_id();

    bool _filter(const sb::update &update, const sb::sources &sources);

    void _sort(std::vector<sb::update> &hist);

    class scuttlebutt : public event_emitter {
    public:
        virtual ~scuttlebutt() {}

        explicit scuttlebutt(scuttlebutt_options opts);

        scuttlebutt &set_id(const std::string &id) {
            id_ = id;
            return *this;
        }

        // each stream will be ended due to this event
        void dispose() {
            emit("dispose");
        }

        int clones() {
            return clones_;
        }

        bool _update(update update);

        dp::duplex_base *create_stream(const stream_options &opts);

        dp::duplex_base *create_write_stream(stream_options &opts);

        dp::duplex_base *create_sink_stream(stream_options &opts);

        dp::duplex_base *create_read_stream(stream_options &opts);

        dp::duplex_base *create_source_stream(stream_options &opts);

    public:
        virtual bool is_accepted(const model_accept &peer_accept, const update &update) = 0;

        virtual bool apply_updates(const update &update) = 0;

        virtual std::vector<update>
        history(const sources &peer_sources, const model_accept &accept) = 0;

    protected:
        bool local_update(const std::pair<std::string, nonstd::any> trx);

    private:
        sign sign_ = nullptr;
        verify verify_ = nullptr;
        int clones_ = 0;

    public:
        int streams = 0;
        sources sources_;
        std::string id_ = "";
        model_accept accept_;
        std::shared_ptr<debug> logger;
    };
} // namespace sb

#endif //SCUTTLEBUTT_SCUTTLEBUTT_H
