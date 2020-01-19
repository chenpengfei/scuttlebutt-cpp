//
// Created by 陈鹏飞 on 2020/1/19.
//

#ifndef SCUTTLEBUTT_SINK_H
#define SCUTTLEBUTT_SINK_H

template <typename T, typename R>
auto log(R read){

    std::function<void (bool, T)> more = [&](bool done, T val){
        if(!done){
            read(false, more);
        }
    };

    read(false, more);
}

#endif //SCUTTLEBUTT_SINK_H
