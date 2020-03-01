//
// Created by 陈鹏飞 on 2020/2/15.
//

#include "pull-stream/include/pull.h"
#include "scuttlebutt.h"
#include "async_model_store.h"
#include "spdlog/spdlog.h"
#include "model.h"

using namespace sb;

void print_key_value(model model, const std::string &k) {
    std::string v = nonstd::any_cast<std::string>(model.get_without_clock<std::string>(k));
    spdlog::info("model id: {}, key: {}, value {}", model.id_, k, v);
}

int main() {
    srand(time(nullptr));

    model a(scuttlebutt_options("A"));
    model b(scuttlebutt_options("B"));

    // in a <-> b relationship, a is read-only and b is write-only
    std::unique_ptr<dp::duplex_pull> s1(a.create_stream(stream_options("a->b")));
    std::unique_ptr<dp::duplex_pull> s2(b.create_stream(stream_options("b->a")));

    a.set("foo", "changed by A");

    std::function<void()> cb = [&b]() {
        print_key_value(b, "foo");
    };
    s2->on("synced", cb);

    link(s1, s2);

//    const clock_t begin_time = clock();
//    int max = 50000;
//    int counter = max;
//    while(counter-- >= 0) {
//        a.set("foo", "changed by A ->" + std::to_string(counter));
//    }
//    auto ms = float(clock() - begin_time) / (CLOCKS_PER_SEC / 1000);
//    spdlog::info("time for {} loop, {}ms", max, ms);

    assert(s1);
    assert(s2);

    return 0;
}

