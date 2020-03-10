//
// Created by 陈鹏飞 on 2020/3/6.
//

#include <regex>
#include "socket-stream.h"
#include "spdlog/spdlog.h"
#include "duplex-stream-to-pull/include/duplex-stream-to-pull.h"
#include "fake_duplex_pull.h"
#include "echo_server.h"
#include "scuttlebutt-pull/scuttlebutt/include/scuttlebutt.h"
#include "scuttlebutt-pull/model/include/model.h"

using namespace pull;
using namespace sb;

void print_key_value(model model, const std::string &k) {
    auto v = model.get_without_clock<std::string>(k);
    spdlog::info("model id: {}, key: {}, value {}", model.id_, k, v);
}

void init() {
#ifdef SPDLOG_ACTIVE_LEVEL
    spdlog::set_level(spdlog::level::level_enum(SPDLOG_ACTIVE_LEVEL));
#endif
    spdlog::set_pattern("[%L][%t] [%Y-%m-%d %H:%M:%S.%e] %v");

    srand(time(nullptr));

    el::START();
}

void sb_test() {
    std::shared_ptr<socket_stream> raw_steam = std::make_shared<socket_stream>(10000);
    if (raw_steam->connect("localhost", "9988") == 0) {
        model a(scuttlebutt_options("A"));
        std::shared_ptr<dp::duplex_pull> s1(a.create_stream(stream_options("a->b")));

        auto cb = [](dp::error err) {
            spdlog::info("end {}", err);
        };
        std::shared_ptr<dp::duplex_pull> s2 = std::make_shared<duplex_stream_to_pull>( raw_steam, cb);

        dp::link(s1, s2);

        for (auto i = 0; i <= 100000; ++i) {
            a.set("foo", "changed by A, value:" + std::to_string(i));
        }

        std::this_thread::sleep_for(std::chrono::seconds(50));
    }
}

void echo_test() {
    echo_server server;
    server.start();

    auto fake_dp = new fake_duplex_pull();

    auto cb = [](dp::error err) {
        spdlog::info("end {}", err);
    };
    std::shared_ptr<socket_stream> raw_steam = std::make_shared<socket_stream>(200);
    if (raw_steam->connect("localhost", "9988") == 0) {
        std::shared_ptr<dp::duplex_pull> s1 = std::make_shared<duplex_stream_to_pull>( raw_steam, cb);
        std::shared_ptr<dp::duplex_pull> s2(fake_dp);

        int n = 100000;
        int sum = 0;
        int counter = 0;
        s2->on("data", std::function<void(int&)>([&counter, &sum](int &n) {
            sum += n;
            ++counter;
//            spdlog::info("counter: {}, n: {}", counter, n);
            assert(counter == n);
        }));

        dp::link(s1, s2);

        for(auto i = 1; i <= n; ++i) {
            fake_dp->push(std::string(std::to_string(i) + "\n"));
        }

        std::this_thread::sleep_for(std::chrono::seconds(20));

        int actual_sum = ((1+n)*n)/2;
        spdlog::info("expected sum:{}, actual sum:{}", sum, actual_sum);
        assert(sum == actual_sum);
    }

    server.stop();
}

int main() {
    init();

    sb_test();

    return 0;
}
