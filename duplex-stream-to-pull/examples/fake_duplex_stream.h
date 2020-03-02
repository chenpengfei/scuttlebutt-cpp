//
// Created by 陈鹏飞 on 2020/3/2.
//

#ifndef SCUTTLEBUTT_FAKE_DUPLEX_STREAM_H
#define SCUTTLEBUTT_FAKE_DUPLEX_STREAM_H

class fake_duplex_stream : public duplex_stream {
public:
    fake_duplex_stream(size_t high_water_mark) : duplex_stream(high_water_mark) {

    }

    bool write(const char *buffer, size_t buffer_len) override {
        auto total_len = w_buffer_len_ + buffer_len;
        if (total_len >= MAX_BUFFER_SIZE) {
            // stream will buffer all written chunks until maximum memory usage occurs,
            // at which point it will abort unconditionally
            return false;
        }

        strncpy(w_buffer_, buffer, buffer_len);
        w_buffer_len_ = total_len;

        return total_len < high_water_mark_;
    }

    size_t read(char *buffer, size_t buffer_len) override {
        auto len = buffer_len <= r_buffer_len_ ? buffer_len : r_buffer_len_;
        strncpy(buffer, r_buffer_, len);
        r_buffer_len_ -= len;
        return len;
    }

    void _destroy() override {
        r_buffer_len_ = 0;
        w_buffer_len_ = 0;
        emit("close");
    }

    void end() override {
        r_buffer_len_ = 0;
        w_buffer_len_ = 0;
        emit("end");
    }

public:
    std::string pop() {
        std::string data(w_buffer_, w_buffer_len_);
        w_buffer_len_ = 0;
        emit("drain");
        return data;
    }

    size_t push(char *buffer, size_t buffer_len) {
        auto total_len = r_buffer_len_ + buffer_len;
        if (total_len >= MAX_BUFFER_SIZE) {
            total_len = MAX_BUFFER_SIZE;
        }
        auto len = total_len - r_buffer_len_;

        strncpy(r_buffer_, buffer, len);
        r_buffer_len_ = total_len;

        emit("readable");

        return len;
    }

private:
    char r_buffer_[MAX_BUFFER_SIZE];
    size_t r_buffer_len_ = 0;

    char w_buffer_[MAX_BUFFER_SIZE];
    size_t w_buffer_len_ = 0;
};

#endif //SCUTTLEBUTT_FAKE_DUPLEX_STREAM_H
