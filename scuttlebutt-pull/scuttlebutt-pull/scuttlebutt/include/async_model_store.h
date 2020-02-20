//
// Created by 陈鹏飞 on 2020/2/16.
//

#ifndef SCUTTLEBUTT_MODEL_STORE_H
#define SCUTTLEBUTT_MODEL_STORE_H

#include <future>
#include "scuttlebutt.h"

namespace sb {

    class async_model_store_base {
    public:
        virtual ~async_model_store_base() = default;

        virtual void init() = 0;
        //todo
    };
} // namespace sb


#endif //SCUTTLEBUTT_MODEL_STORE_H
