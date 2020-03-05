//
// Created by 陈鹏飞 on 2020/3/4.
//

#ifndef SCUTTLEBUTT_PULL_SPLIT_H
#define SCUTTLEBUTT_PULL_SPLIT_H

#include <regex>
#include <iostream>
#include <utility>
#include "pull-through/include/pull-through.h"

namespace pull {

    using mapper = std::function<std::string(const std::string &)>;

    class pull_split final {
    public:
        virtual ~pull_split() = default;

        // std::regex ws_re(R"(\n|($(?!\s)))"); may be better.todo
        pull_split(const std::string &matcher = "\n|$",
                   mapper mapper = nullptr,
                   bool reverse = false,
                   bool last = false) :
                matcher_(matcher), mapper_(std::move(mapper)), reverse_(reverse), last_(last) {
            auto self = this;
            through_ = pull_through([self](const std::string &buffer) {
                auto s = (self->reverse_ ? buffer + self->so_far_ : self->so_far_ + buffer);
                std::vector<std::string> pieces{
                        std::sregex_token_iterator(s.begin(), s.end(), self->matcher_, -1), {}
                };
                self->so_far_ = self->reverse_ ? pieces.front() : pieces.back();
                auto l = pieces.size();
                for (auto i = 0; i < l - 1; ++i) {
                    self->map(pieces[self->reverse_ ? l - 1 - i : i]);
                }
            }, [self]() {
                if (self->last_ && self->so_far_ == "") {
                    return self->through_.enqueue("");
                }
                self->map(self->so_far_);
                self->through_.enqueue("");
            });
        }

        dp::read operator()(dp::read read) {
            return through_(std::move(read));
        }

    private:
        void map(const std::string &piece) {
            if (mapper_) {
                through_.enqueue(mapper_(piece));
            } else {
                through_.enqueue(piece);
            }
        }

    private:
        pull_through through_;

        std::regex matcher_;
        mapper mapper_ = nullptr;
        bool reverse_ = false;
        bool last_ = false;
        std::string so_far_ = "";
    };
}

#endif //SCUTTLEBUTT_PULL_SPLIT_H
