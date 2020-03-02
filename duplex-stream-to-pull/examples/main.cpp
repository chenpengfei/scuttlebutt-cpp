//
// Created by 陈鹏飞 on 2020/1/19.

#include <iostream>
#include "duplex-stream-to-pull/include/duplex-stream-to-pull.h"
#include "spdlog/spdlog.h"
#include "duplex-pull/include/duplex-pull.h"
#include "fake_duplex_pull.h"
#include "fake_duplex_stream.h"

using namespace std;


int main() {
    char a_buffer[6] = {'1', '2', '2', '3', '3', '3'};
    char b_buffer[6] = {'a', 'b', 'b', 'c', 'c', 'c'};

    auto cb = [](dp::error err) {
        spdlog::info("end {}", err);
    };
    std::shared_ptr<fake_duplex_stream> raw_steam = std::make_shared<fake_duplex_stream>(3);
    std::unique_ptr<dp::duplex_pull> s1 = std::make_unique<duplex_stream_to_pull>( raw_steam, cb);

    auto md = new fake_duplex_pull();
    std::unique_ptr<dp::duplex_pull> s2(md);

    dp::link(s1, s2);

    raw_steam->push(a_buffer, 1);
    spdlog::info(md->pop()); // 1
    raw_steam->push(a_buffer + 1, 2);
    spdlog::info(md->pop()); // 22
    raw_steam->push(a_buffer + 3, 3);
    spdlog::info(md->pop()); // 333

    md->push(b_buffer, 1);
    spdlog::info(raw_steam->pop()); // a
    md->push(b_buffer + 1, 2);
    spdlog::info(raw_steam->pop()); // bb
    md->push(b_buffer + 3, 3);
    spdlog::info(raw_steam->pop()); // ccc

    return 0;
}


