//
// Created by 陈鹏飞 on 2020/2/25.
//

#include "gtest/gtest.h"
#include "model.h"

using namespace sb;
using namespace std;

struct kv {
    string key;
    string valueA;
    string valueB;
};

const model_accept accept{vector<string>{"foo"}};
const kv expected{"foo", "changed by A", "changed by B"};
const kv ignored{"ignored", "changed by A", "changed by B"};

TEST(whitelist_filter_out_in_history, model_filter) {
    model a(scuttlebutt_options("A"));
    model b(scuttlebutt_options{"B", accept});

    std::unique_ptr<dp::duplex_pull> s1(a.create_stream(stream_options("a->b")));
    std::unique_ptr<dp::duplex_pull> s2(b.create_stream(stream_options("b->a")));

    a.set(expected.key, expected.valueA);
    a.set(ignored.key, expected.valueA);

    s2->on("synced", std::function<void()>([&]() {
        ASSERT_EQ(expected.valueA, b.get_without_clock<std::string>(expected.key));
        ASSERT_EQ("", b.get_without_clock<std::string>(ignored.key));
    }));

    link(s1, s2);
}

TEST(whitelist_filter_out_in_following_update, model_filter) {
    model a(scuttlebutt_options("A"));
    model b(scuttlebutt_options{"B", accept});

    std::unique_ptr<dp::duplex_pull> s1(a.create_stream(stream_options("a->b")));
    std::unique_ptr<dp::duplex_pull> s2(b.create_stream(stream_options("b->a")));

    link(s1, s2);

    a.set(expected.key, expected.valueA);
    a.set(ignored.key, expected.valueA);

    ASSERT_EQ(expected.valueA, a.get_without_clock<std::string>(expected.key));
    ASSERT_EQ(ignored.valueA, a.get_without_clock<std::string>(ignored.key));

    ASSERT_EQ(expected.valueA, b.get_without_clock<std::string>(expected.key));
    ASSERT_EQ("", b.get_without_clock<std::string>(ignored.key));
}


