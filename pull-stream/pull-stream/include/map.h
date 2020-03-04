//
// Created by 陈鹏飞 on 2020/1/19.
//

#ifndef SCUTTLEBUTT_THROUGH_H
#define SCUTTLEBUTT_THROUGH_H

namespace pull {

    template<typename T, typename M>
    decltype(auto) Map(M &&mapper) {

        return [&mapper](auto &&read) {
            return [read, &mapper](dp::error abort, auto cb) {
                read(abort, [&](dp::error end, T val) {
                    dp::error ended = dp::end_or_err(abort) ? abort : end;

                    if (dp::end_or_err(ended)) {
                        cb(ended, val);
                    } else {
                        cb(dp::error::ok, mapper(val));
                    }
                });
            };
        };
    }
}

#endif //SCUTTLEBUTT_THROUGH_H
