//
// Created by 陈鹏飞 on 2020/2/29.
//

#ifndef SCUTTLEBUTT_DUPLEX_STREAM_TO_PULL_H
#define SCUTTLEBUTT_DUPLEX_STREAM_TO_PULL_H

#include <utility>

#include "duplex-pull/include/duplex-pull.h"
#include "duplex-stream/include/duplex-stream.h"
#include "pull-looper/include/pull_looper.h"

const size_t MAX_READ_SIZE = (32 * 1024);

class duplex_stream_to_pull : public dp::duplex_pull {
public:
    ~duplex_stream_to_pull() override = default;

    explicit duplex_stream_to_pull(std::shared_ptr<duplex_stream> stream,
                                   std::function<void(dp::error)> cb = nullptr);

    dp::read &source() override;

    dp::sink &sink() override;

    void end() override;

private:
    std::shared_ptr<duplex_stream> stream_;

private:
    // write/sink
    dp::error w_ended_ = dp::error::ok;
    bool w_closed_ = false;
    bool w_did_ = false;
    std::function<void(dp::error)> w_end_cb_ = nullptr;

    dp::source_callback w_done_ = nullptr;
    std::function<void()> w_on_close_ = nullptr;
    std::function<void(dp::error)> w_on_error_ = nullptr;
    std::function<void()> w_cleanup_ = nullptr;

    dp::sink raw_sink_ = nullptr;
    dp::read peer_read_ = nullptr;
    dp::source_callback more_ = nullptr;
    pull::looper looper_;
    std::function<void()> looper_next_ = nullptr;

    // read/source
    dp::error r_ended_ = dp::error::ok;
    bool r_waiting_ = false;
    dp::source_callback r_cb_ = nullptr;

    std::function<void()> r_read_ = nullptr;
    std::function<void()> r_on_readable_ = nullptr;
    std::function<void()> r_on_end_ = nullptr;
    std::function<void(dp::error)> r_on_error_ = nullptr;

    char r_buffer_[MAX_READ_SIZE]{};

    dp::read raw_source_ = nullptr;
};


#endif //SCUTTLEBUTT_DUPLEX_STREAM_TO_PULL_H
