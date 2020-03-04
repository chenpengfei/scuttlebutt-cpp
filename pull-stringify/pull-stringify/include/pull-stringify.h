//
// Created by 陈鹏飞 on 2020/3/2.
//

#ifndef SCUTTLEBUTT_PULL_STRINGIFY_H
#define SCUTTLEBUTT_PULL_STRINGIFY_H

#include <iostream>
#include <utility>
#include "duplex-pull/include/duplex-pull.h"
#include "nlohmann/json.hpp"

struct pull_stringify_options {
    std::string open_ = "";
    std::string prefix_ = "";
    std::string suffix = "\n";
    std::string close_ = "";
    int indent_ = 2;
};

class pull_stringify final {
public:
    explicit pull_stringify(pull_stringify_options opts = pull_stringify_options{})
        : opts_(std::move(opts)) {
    }

    dp::through serialize() {
        auto self = this;
        return  [self](dp::read read) {
            self->peer_read_ = std::move(read);

            return [self](dp::error end, dp::source_callback cb) {
                self->cb_ = std::move(cb);

                if (dp::end_or_err(self->ended_) || dp::end_or_err(end)) {
                    return self->cb_(dp::end_or_err(self->ended_)? self->ended_ : end, nullptr);
                }

                self->peer_read_(dp::error::ok, [self](dp::error end, const nlohmann::json &data) {
                    if (dp::error::ok == end) {
                        auto f = self->first_;
                        self->first_ = false;

                        auto serialized = data.dump(self->opts_.indent_);
                        auto str = (f? self->opts_.open_ : self->opts_.prefix_) + serialized + self->opts_.suffix;
                        self->cb_(dp::error::ok, str);
                    } else {
                        self->ended_ = end;
                        if (self->ended_ != dp::error::end) {
                            return self->cb_(self->ended_, nullptr);
                        }
                        self->cb_(dp::error::ok, self->first_? self->opts_.open_ + self->opts_.close_ : self->opts_.close_);
                    }
                });
            };
        };
    }

private:
    pull_stringify_options opts_;

    bool first_ = true;
    dp::error ended_ = dp::error::ok;

    dp::read peer_read_ = nullptr;
    dp::source_callback cb_ = nullptr;
};

#endif //SCUTTLEBUTT_PULL_STRINGIFY_H
