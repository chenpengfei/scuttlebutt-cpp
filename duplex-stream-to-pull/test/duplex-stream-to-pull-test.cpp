//
// Created by 陈鹏飞 on 2020/2/20.
//

#include "gtest/gtest.h"
#include "duplex-stream-to-pull.h"
#include "pull-stream/include/values.h"
#include "pull-stream/include/pull.h"
#include "gmock/gmock.h"
#include "spdlog/spdlog.h"

class mock_duplex_stream : public duplex_stream {
public:
    mock_duplex_stream(size_t high_water_mark) : duplex_stream(high_water_mark) {

    }

    MOCK_METHOD2(write_, bool(const char *buffer, size_t buffer_len));
    virtual bool write(const char *buffer, size_t buffer_len) override {
        return write_(buffer, buffer_len);
    };

    MOCK_METHOD2(read_, size_t(char *buffer, size_t buffer_len));
    virtual size_t read(char *buffer, size_t buffer_len) override {
        char buffer_[3] = {'1', '2', '3'};
        strncpy(buffer, buffer_, 1);
        return 1;
    }

    MOCK_METHOD0(end_, void());
    virtual void end() override  {
        end_();
    }

    MOCK_METHOD0(destroy_, void());
    virtual void _destroy() override {
        destroy_();
    }
};

TEST(abort_, duplex_stream_to_pull) {
    mock_duplex_stream *mock_stream = new mock_duplex_stream(1024);
    std::shared_ptr<duplex_stream> ds(mock_stream);
    std::shared_ptr<dp::duplex_pull> s1 = std::make_shared<duplex_stream_to_pull>(ds);

    int counter = 0;

    auto cb = dp::source_callback([&counter](dp::error, const nonstd::any & data){
        std::string str = nonstd::any_cast<std::string>(data);
        ASSERT_EQ("1", str);
        ++counter;
    });

    auto sink = dp::sink([&cb](const dp::read& read){
        read(dp::error::ok, cb);
    });

    mock_stream->emit("readable");

    pull::pull(s1->source(), sink);

    ASSERT_EQ(1, counter);
}

