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

    MOCK_METHOD2(write, bool(
            const char *buffer, size_t
            buffer_len));

    MOCK_METHOD2(read, size_t(char * buffer, size_t
            buffer_len));

    MOCK_METHOD0(end, void());

    MOCK_METHOD0(_destroy, void());
};

TEST(read_data_from_stream, duplex_stream_to_pull) {
    auto *mock_stream = new mock_duplex_stream(1024);
    std::shared_ptr<duplex_stream> ds(mock_stream);
    std::shared_ptr<dp::duplex_pull> s1 = std::make_shared<duplex_stream_to_pull>(ds);

    int counter = 0;

    auto cb = dp::source_callback([&counter](dp::error, const nonstd::any &data) {
        std::string str = nonstd::any_cast<std::string>(data);
        ASSERT_EQ("1", str);
        ++counter;
    });

    auto sink = dp::sink([&cb](const dp::read &read) {
        read(dp::error::ok, cb);
    });

    char buffer_[3] = {'1', '2', '3'};
    EXPECT_CALL(*mock_stream, read(_, _)).WillOnce(Invoke(
            [&buffer_](char *buffer, size_t buffer_len) {
                strncpy(buffer, buffer_, 1);
                return 1;
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
    EXPECT_CALL(*mock_stream, read(_, _))
            .Times(3)
            .WillOnce(Invoke([&r_buffer_](char *buffer, size_t buffer_len) {
                strncpy(buffer, r_buffer_, 1);
                spdlog::info("hello 1");
                return 1;
            }))
            .WillOnce(Invoke([&r_buffer_](char *buffer, size_t buffer_len) {
                strncpy(buffer, r_buffer_ + 1, 2);
                spdlog::info("hello 2");
                return 2;
            }))
            .WillOnce(Invoke([&r_buffer_](char *buffer, size_t buffer_len) {
                spdlog::info("hello 3");
                return 0;
            }));

    char w_buffer_[3];
    size_t w_len = 0;
    EXPECT_CALL(*mock_stream, write(_, _))
            .Times(2)
            .WillRepeatedly(Invoke([&w_buffer_, &w_len](const char *buffer, size_t buffer_len) {
                strncpy(w_buffer_ + w_len, buffer, buffer_len);
                w_len += buffer_len;
                return true;
            }));

    mock_stream->emit("readable");

    pull::pull(s1->source(), s1->sink());

    ASSERT_EQ(std::string(r_buffer_, 3), std::string(w_buffer_, 3));
}