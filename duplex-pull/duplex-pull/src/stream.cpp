//
// Created by 陈鹏飞 on 2020/2/20.
//

#include "duplex-pull.h"
#include "pull-stream/include/pull.h"
#include "event-loop/include/event-loop.h"

namespace dp {

    bool end_or_err(error err) {
        return error::end == err || error::err == err;
    }

    void link(duplex_pull *a, duplex_pull *b) {
        el::PUSH(el::handler([a, b](){
            pull::pull(a->source(), b->sink());
            pull::pull(b->source(), a->sink());
        }));
    }

    void link(const std::shared_ptr<duplex_pull> &a, const std::shared_ptr<duplex_pull> &b) {
        el::PUSH(el::handler([a, b](){
            pull::pull(a->source(), b->sink());
            pull::pull(b->source(), a->sink());
        }));
    }
}
