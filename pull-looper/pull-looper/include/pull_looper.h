//
// Created by 陈鹏飞 on 2020/2/26.
//

#ifndef SCUTTLEBUTT_PULL_LOOPER_H
#define SCUTTLEBUTT_PULL_LOOPER_H

#include <iostream>

namespace pull {

    std::function<void()> looper(std::function<void()> f);
}

#endif //SCUTTLEBUTT_PULL_LOOPER_H
