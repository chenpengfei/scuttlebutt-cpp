//
// Created by 陈鹏飞 on 2020/2/25.
//

#include "gtest/gtest.h"
#include "model.h"
#include "event_listener.h"

using namespace std;
using namespace sb;

struct kv {
    string key;
    string valueA;
    string valueB;
};

const kv expected{"foo", "changed by A", "changed by B"};

TEST(local_change, model) {
    model a(scuttlebutt_options("A"));

    auto c = 2;
    a.on("changed", changed_with_kv_listener([&](std::string &key, nonstd::any &value) {
        ASSERT_EQ(expected.key, key);
        ASSERT_EQ(expected.valueA, nonstd::any_cast<std::string>(value));
        --c;
    }));

    a.on("changed:" + expected.key, changed_with_v_listener([&](nonstd::any &value) {
        ASSERT_EQ(expected.valueA, nonstd::any_cast<std::string>(value));
        --c;
    }));

    a.set(expected.key, expected.valueA);

    ASSERT_EQ(0, c);
}

TEST(change_before_sync, model) {
    model a(scuttlebutt_options("A"));
    model b(scuttlebutt_options{"B"});

    std::unique_ptr<dp::duplex_pull> s1(a.create_stream(stream_options("a->b")));
    std::unique_ptr<dp::duplex_pull> s2(b.create_stream(stream_options("b->a")));

    a.set(expected.key, expected.valueA);

    s2->on("synced", std::function<void()>([&]() {
        ASSERT_EQ(expected.valueA, b.get_without_clock<std::string>(expected.key));
    }));

    link(s1, s2);
}

TEST(change_after_sync, model) {
    model a(scuttlebutt_options("A"));
    model b(scuttlebutt_options{"B"});

    std::unique_ptr<dp::duplex_pull> s1(a.create_stream(stream_options("a->b")));
    std::unique_ptr<dp::duplex_pull> s2(b.create_stream(stream_options("b->a")));

    b.on("changedByPeer",
         changed_by_peer_listener([&](std::string &key, nonstd::any &value, const sb::from &from) {
             ASSERT_EQ(expected.key, key);
             ASSERT_EQ(expected.valueA, nonstd::any_cast<std::string>(value));
             ASSERT_EQ(expected.valueA, b.get_without_clock<std::string>(expected.key));
         }));

    link(s1, s2);
    a.set(expected.key, expected.valueA);
}

TEST(change_in_two_ways, model) {
    model a(scuttlebutt_options("A"));
    model b(scuttlebutt_options{"B"});

    std::unique_ptr<dp::duplex_pull> s1(a.create_stream(stream_options("a->b")));
    std::unique_ptr<dp::duplex_pull> s2(b.create_stream(stream_options("b->a")));

    a.set(expected.key, expected.valueA);

    b.on("changedByPeer",
         changed_by_peer_listener([&](std::string &key, nonstd::any &value, const sb::from &from) {
             ASSERT_EQ(a.id_, from);
             ASSERT_EQ(expected.key, key);
             ASSERT_EQ(expected.valueA, nonstd::any_cast<std::string>(value));
             ASSERT_EQ(expected.valueA, b.get_without_clock<std::string>(expected.key));

             a.on("changedByPeer", changed_by_peer_listener(
                     [&](std::string &key, nonstd::any &value, const sb::from &from) {
                         ASSERT_EQ(b.id_, from);
                         ASSERT_EQ(expected.key, key);
                         ASSERT_EQ(expected.valueB, nonstd::any_cast<std::string>(value));
                         ASSERT_EQ(expected.valueB, a.get_without_clock<std::string>(expected.key));
                     }));
             b.set(expected.key, expected.valueB);
         }));

    link(s1, s2);
}
