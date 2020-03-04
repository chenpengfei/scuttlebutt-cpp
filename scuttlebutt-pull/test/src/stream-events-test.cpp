//
// Created by 陈鹏飞 on 2020/2/23.
//

#include <typeinfo>
#include "gtest/gtest.h"
#include "duplex-pull/include/duplex-pull.h"
#include "model.h"
#include "event_listener.h"

using namespace std;
using namespace sb;

TEST(update_sent_received_event, stream) {
    model a(scuttlebutt_options("A"));
    model b(scuttlebutt_options("B"));

    std::unique_ptr<dp::duplex_pull> s1(a.create_stream(stream_options("a->b")));
    std::unique_ptr<dp::duplex_pull> s2(b.create_stream(stream_options("b->a")));

    int s1_update_sent_fired_counter = 0;
    update_sent_listener s1_update_sent_fired = [&](dp::duplex_pull *duplex,
                                                    const nlohmann::json &update,
                                                    int &sent_counter, std::string id_name) {
        ASSERT_EQ(s1.get(), duplex);
        ASSERT_TRUE(update.is_array());
        ASSERT_EQ(a.id_ + "/" + s1->name(), id_name);
        ++s1_update_sent_fired_counter;
    };
    int s1_update_received_fired_counter = 0;
    update_received_listener s1_update_received_fired = [&](dp::duplex_pull *duplex,
                                                            const nlohmann::json &update,
                                                            int &sent_counter,
                                                            std::string id_name) {
        ++s1_update_received_fired_counter;
    };

    int s2_update_sent_fired_counter = 0;
    update_sent_listener s2_update_sent_fired = [&](dp::duplex_pull *duplex,
                                                    const nlohmann::json &update,
                                                    int &sent_counter, std::string id_name) {
        ++s2_update_sent_fired_counter;
    };
    int s2_update_received_fired_counter = 0;
    update_received_listener s2_update_received_fired = [&](dp::duplex_pull *duplex,
                                                            const nlohmann::json &update,
                                                            int &sent_counter,
                                                            std::string id_name) {
        ASSERT_EQ(s2.get(), duplex);
        ASSERT_TRUE(update.is_array());
        ASSERT_EQ(b.id_ + "/" + s2->name(), id_name);
        ++s2_update_received_fired_counter;
    };

    s1->on("updateSent", s1_update_sent_fired);
    s1->on("updateReceived", s1_update_received_fired);

    s2->on("updateSent", s2_update_sent_fired);
    s2->on("updateReceived", s2_update_received_fired);

    a.set("foo", "changed by A");

    link(s1, s2);

    ASSERT_EQ(1, s1_update_sent_fired_counter);
    ASSERT_EQ(0, s1_update_received_fired_counter);

    ASSERT_EQ(0, s2_update_sent_fired_counter);
    ASSERT_EQ(1, s2_update_received_fired_counter);
}