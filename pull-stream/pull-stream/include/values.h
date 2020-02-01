//
// Created by 陈鹏飞 on 2020/1/19.
//

#ifndef SCUTTLEBUTT_SOURCE_H
#define SCUTTLEBUTT_SOURCE_H

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

#endif //SCUTTLEBUTT_SOURCE_H
