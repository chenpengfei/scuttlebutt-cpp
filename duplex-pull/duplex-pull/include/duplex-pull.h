//
// Created by 陈鹏飞 on 2020/2/20.
//

#ifndef SCUTTLEBUTT_DUPLEX_PULL_H
#define SCUTTLEBUTT_DUPLEX_PULL_H

#include <iostream>
#include "nonstd/any.h"
#include "event-emitter/include/event_emitter.h"

namespace dp {

    using source_callback = std::function<void(bool, const nonstd::any&)>;

    using read = std::function<void(bool, source_callback)>;

    using sink = std::function<void(read)>;

    class duplex_base : public event_emitter {
    public:
        virtual read &source() = 0;
        virtual sink &sink() = 0;
    };

    void link(duplex_base *a, duplex_base *b);
}

#endif //SCUTTLEBUTT_DUPLEX_PULL_H
