//
// Created by 陈鹏飞 on 2020/2/24.
//

#include "gtest/gtest.h"
#include "duplex-pull/include/duplex-pull.h"
#include "model.h"
#include "scuttlebutt.h"
#include "event_listener.h"

using namespace sb;

TEST(end_one_stream, stream) {
    model a(scuttlebutt_options("A"));
    model b(scuttlebutt_options("B"));

    std::unique_ptr<dp::duplex_base> s1(a.create_stream(stream_options("a->b")));
    std::unique_ptr<dp::duplex_base> s2(b.create_stream(stream_options("b->a")));

    link(s1, s2);

    ASSERT_EQ(1, a.listener_count("_update"));
    ASSERT_EQ(1, b.listener_count("_update"));

    s1->end();

    ASSERT_EQ(0, s1->listener_count("_update"));
    ASSERT_EQ(0, s2->listener_count("_update"));
}

TEST(stream_count_when_end, stream) {
    model a(scuttlebutt_options("A"));
    model b(scuttlebutt_options("B"));

    std::unique_ptr<dp::duplex_base> s1(a.create_stream(stream_options("a->b")));
    std::unique_ptr<dp::duplex_base> s2(b.create_stream(stream_options("b->a")));

    ASSERT_EQ(1, a.streams);
    ASSERT_EQ(1, b.streams);

    link(s1, s2);

    a.on("unstream", unstream_listener([](int &count){
        ASSERT_EQ(0, count);
    }));
    b.on("unstream", unstream_listener([](int &count){
        ASSERT_EQ(0, count);
    }));
    s1->end();
}

TEST(stream_count_when_dispose, stream) {
    model a(scuttlebutt_options("A"));

    std::unique_ptr<dp::duplex_base> s1(a.create_stream(stream_options("s1")));
    std::unique_ptr<dp::duplex_base> s2(a.create_stream(stream_options("s2")));

    ASSERT_EQ(2, a.streams);

    int counter = 2;
    a.on("unstream", unstream_listener([&](int &count){
        --counter;
    }));

    a.dispose();

    ASSERT_EQ(0, counter);
}