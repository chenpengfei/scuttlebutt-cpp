//
// Created by 陈鹏飞 on 2020/2/20.
//

#ifndef SCUTTLEBUTT_DUPLEX_PULL_H
#define SCUTTLEBUTT_DUPLEX_PULL_H

#include <iostream>
#include "nonstd/any.h"
#include "event-emitter/include/event_emitter.h"
#include "nlohmann/json.hpp"

namespace dp {

    enum error {
        ok = 0,
        end,
        err,
    };

    bool end_or_err(error err);

    using source_callback = std::function<void(error, const nlohmann::json &)>;

    using read = std::function<void(error, source_callback)>;

    using sink = std::function<void(read)>;

    using through = std::function<read(read)>;

    class duplex_pull : public event_emitter {
    public:
        ~duplex_pull() override = default;

        virtual read &source() = 0;

        virtual sink &sink() = 0;

        virtual void end() = 0;

        std::string name() { return name_; }

        bool readable() { return readable_; }

        void readable(bool value) { readable_ = value; }

        bool writable() { return writable_; }

        void writable(bool value) { writable_ = value; }

    protected:
        std::string name_ = "stream";
        bool readable_ = true;
        bool writable_ = true;
    };

    void link(duplex_pull *a, duplex_pull *b);

    void link(const std::shared_ptr<duplex_pull> &a, const std::shared_ptr<duplex_pull> &b);
}

#endif //SCUTTLEBUTT_DUPLEX_PULL_H
