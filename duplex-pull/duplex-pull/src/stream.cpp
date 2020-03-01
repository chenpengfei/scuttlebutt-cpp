//
// Created by 陈鹏飞 on 2020/2/20.
//

#include "duplex-pull.h"
#include "pull-stream/include/pull.h"

namespace dp {

    bool end_or_err(error err) {
        return error::end == err || error::err == err;
    }

    void link(duplex_pull *a, duplex_pull *b) {
        pull::pull(a->source(), b->sink());
        pull::pull(b->source(), a->sink());
    }

    void link(const std::unique_ptr<duplex_pull> &a, const std::unique_ptr<duplex_pull> &b) {
        pull::pull(a->source(), b->sink());
        pull::pull(b->source(), a->sink());
    }
}
