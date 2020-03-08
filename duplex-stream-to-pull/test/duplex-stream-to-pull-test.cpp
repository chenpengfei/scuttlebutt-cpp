//
// Created by 陈鹏飞 on 2020/2/20.
//

#include "gtest/gtest.h"
#include "duplex-stream-to-pull.h"
#include "pull-stream/include/pull.h"
#include "gmock/gmock.h"
#include "spdlog/spdlog.h"

using ::testing::Mock;
using ::testing::_;
using ::testing::Invoke;

class mock_duplex_stream : public duplex_stream {
public:
    mock_duplex_stream(size_t high_water_mark) : duplex_stream(high_water_mark) {}

    MOCK_METHOD1(write, bool(const std::string &));

    MOCK_METHOD0(read, std::string());

    MOCK_METHOD0(end, void());

    MOCK_METHOD0(_destroy, void());
};

TEST(read_data_from_stream, duplex_stream_to_pull) {
    auto *mock_stream = new mock_duplex_stream(1024);
    std::shared_ptr<duplex_stream> ds(mock_stream);
    std::shared_ptr<dp::duplex_pull> s1 = std::make_shared<duplex_stream_to_pull>(ds);

    int counter = 0;

    auto cb = dp::source_callback([&counter](dp::error, const nlohmann::json &data) {
        std::string str = data.get<std::string>();
        ASSERT_EQ("1", str);
        ++counter;
    });

    auto sink = dp::sink([&cb](const dp::read &read) {
        read(dp::error::ok, cb);
    });

    char buffer_[3] = {'1', '2', '3'};
    EXPECT_CALL(*mock_stream, read()).WillOnce(Invoke(
            [&buffer_]() {
                return buffer_[0];
            }));

    mock_stream->emit("readable");

    pull::pull(s1->source(), sink);

    ASSERT_EQ(1, counter);
}

TEST(source_to_sink, duplex_stream_to_pull) {
    auto *mock_stream = new mock_duplex_stream(1024);
    std::shared_ptr<duplex_stream> ds(mock_stream);
    std::shared_ptr<dp::duplex_pull> s1 = std::make_shared<duplex_stream_to_pull>(ds);

    char r_buffer_[3] = {'1', '2', '3'};
    EXPECT_CALL(*mock_stream, read())
            .Times(3)
            .WillOnce(Invoke([&r_buffer_]() {
                return r_buffer_[0];
            }))
            .WillOnce(Invoke([&r_buffer_]() {
                return std::string(&r_buffer_[1], 2);
            }))
            .WillOnce(Invoke([&r_buffer_]() {
                return std::string();
            }));

    char w_buffer_[3];
    size_t w_len = 0;
    EXPECT_CALL(*mock_stream, write(_))
            .Times(2)
            .WillRepeatedly(Invoke([&w_buffer_, &w_len](const std::string &data) {
                strncpy(w_buffer_ + w_len, data.c_str(), data.length());
                w_len += data.length();
                return true;
            }));

    mock_stream->emit("readable");

    pull::pull(s1->source(), s1->sink());

    ASSERT_EQ(std::string(r_buffer_, 3), std::string(w_buffer_, 3));
}