//
// Created by 陈鹏飞 on 2020/2/26.
//

#ifndef SCUTTLEBUTT_PULL_LOOPER_H
#define SCUTTLEBUTT_PULL_LOOPER_H

#include <iostream>

namespace pull {

    class looper {
    public:
        virtual ~looper() {}

        std::function<void()> operator()(std::function<void()>);

    private:
        bool active = false;
        bool called = false;
    };
}

#endif //SCUTTLEBUTT_PULL_LOOPER_H
