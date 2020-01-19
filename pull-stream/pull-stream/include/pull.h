//
// Created by 陈鹏飞 on 2020/1/19.
//

#ifndef SCUTTLEBUTT_PULL_H
#define SCUTTLEBUTT_PULL_H

template <typename T>
T pull(T &&stream){
    return stream;
}

// return void/read
// read -> sink/through
template <typename R, typename T>
auto pull(R &&read, T &&sink){
    return sink(read);
}

// return read
// read -> through -> ...
template <typename R, typename T, typename... Ts>
auto pull(R &&read, T &&through, Ts &&... args){
    return pull(through(read), args...);
}

// return through/sink
// through -> through/sink
template <typename T, typename R, typename... Ts>
auto pull2(T &&through1, R &&through2, Ts &&... args){

    return [&](auto &&read) {
        return pull(read, through1, through2, args...);
    };
}

#endif //SCUTTLEBUTT_PULL_H
