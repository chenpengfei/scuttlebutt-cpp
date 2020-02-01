//
// Created by 陈鹏飞 on 2020/1/19.
//

#ifndef SCUTTLEBUTT_PULL_H
#define SCUTTLEBUTT_PULL_H

template<typename T>
decltype(auto) pull(T &&stream) {
    return stream;
}

// return void/read
// read -> sink/through
template<typename R, typename T>
decltype(auto) pull(R &&read, T &&sink) {
    return sink(read);
}

// return read
// read -> through -> ...
template<typename R, typename T, typename... Ts>
decltype(auto) pull(R &&read, T &&through, Ts &&... args) {
    return pull(through(read), std::forward<Ts>(args)...);
}

// return through/sink
// through -> through/sink
template<typename T, typename R, typename... Ts>
decltype(auto) pipe_through(T &&through1, R &&through2, Ts &&... args) {

    return [&](auto &&read) {
        return pull(read, through1, through2, std::forward<Ts>(args)...);
    };
}

#endif //SCUTTLEBUTT_PULL_H
