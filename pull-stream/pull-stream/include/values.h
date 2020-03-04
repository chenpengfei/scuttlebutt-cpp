//
// Created by 陈鹏飞 on 2020/1/19.
//

#ifndef SCUTTLEBUTT_SOURCE_H
#define SCUTTLEBUTT_SOURCE_H

#include "duplex-pull/include/duplex-pull.h"

namespace pull {

    template<typename T>
    decltype(auto) values(T &begin, T &end) {

        return [&begin, &end](dp::error abort, auto cb) {

            if (dp::end_or_err(abort) || (begin == end)) {
                cb(dp::error::end, 0);
            } else {
                cb(dp::error::ok, *begin++);
            }
        };
    }

    template<typename T>
    decltype(auto) values(T &value) {

        return [&value](dp::error abort, auto cb) {

            if (dp::end_or_err(abort) || (value <= 0)) {
                cb(dp::error::end, -1);
            } else {
                cb(dp::error::ok, value--);
            }
        };
    }
}

#endif //SCUTTLEBUTT_SOURCE_H
