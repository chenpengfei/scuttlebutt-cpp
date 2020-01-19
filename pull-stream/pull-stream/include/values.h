//
// Created by 陈鹏飞 on 2020/1/19.
//

#ifndef SCUTTLEBUTT_SOURCE_H
#define SCUTTLEBUTT_SOURCE_H

template <typename T>
auto values(T &&begin, T &&end) {

    return [&](bool abort, auto cb){

        if(begin != end){
            cb(false, *begin++);
        }
        else{
            cb(true, *begin);
        }
    };
}

#endif //SCUTTLEBUTT_SOURCE_H
