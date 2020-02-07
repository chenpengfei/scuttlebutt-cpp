//
// Created by 陈鹏飞 on 2020/1/19.
//

#ifndef SCUTTLEBUTT_THROUGH_H
#define SCUTTLEBUTT_THROUGH_H

template<typename T, typename M>
decltype(auto) Map(M &&mapper) {

    return [&mapper](auto &&read) {
        return [read, &mapper](bool abort, auto cb) {
            read(abort, [&](bool end, T val) {
                bool ended = abort || end;
                if (ended)
                    cb(true, val);
                else
                    cb(false, mapper(val));
            });
        };
    };
}

#endif //SCUTTLEBUTT_THROUGH_H
