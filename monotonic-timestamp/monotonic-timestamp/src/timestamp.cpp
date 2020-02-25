//
// Created by 陈鹏飞 on 2020/2/19.
//

#include <chrono>

namespace monotonic_timestamp {
    double _last = 0;;
    double _count = 1;
    double adjusted = 0;
    double _adjusted = 0;

    inline double now() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    double timestamp() {
        auto time = now();

        /**
          If time returned is same as in last call, adjust it by
          adding a number based on the counter.
          Counter is incremented so that next call get's adjusted properly.
          Because floats have restricted precision,
          may need to step past some values...
          **/
        if (_last == time) {
            do {
                ++_count;
                adjusted = time + ((_count) / (_count + 999));
            } while (adjusted == _adjusted);
            _adjusted = adjusted;
        }
            // If last time was different reset timer back to `1`.
        else {
            _count = 1;
            adjusted = time;
        }
        _adjusted = adjusted;
        _last = time;
        return adjusted;
    }
}


