//
// Created by 陈鹏飞 on 2020/2/19.
//

#include <chrono>
#include <thread>

namespace monotonic_timestamp {
    long _last = 0;

    inline long now() {
        return std::chrono::duration_cast< std::chrono::milliseconds >(
                std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    long timestamp() {
        long time = now();

        if (_last == time)  {
            do {
                std::this_thread::sleep_for(std::chrono::milliseconds (1));
                time = now();
            } while (_last == time);
        }

        _last = time;
        return time;
    }
}


