//
// Created by 陈鹏飞 on 2020/1/19.
//

#ifndef SCUTTLEBUTT_SOURCE_H
#define SCUTTLEBUTT_SOURCE_H

namespace pull {

    template<typename T>
    decltype(auto) values(T &begin, T &end) {

        return [&begin, &end](bool abort, auto cb) {

            if (abort || (begin == end)) {
                cb(true, -1);
            } else {
                cb(false, *begin++);
            }
        };
    }

    template <typename T>
    decltype(auto) values(T &value) {

        return [&value](bool abort, auto cb) {

            if (abort || (value <= 0)) {
                cb(true, -1);
            } else {
                cb(false, value--);
            }
        };
    }
}

#endif //SCUTTLEBUTT_SOURCE_H
