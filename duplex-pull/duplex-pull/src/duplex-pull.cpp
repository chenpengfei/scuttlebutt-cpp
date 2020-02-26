//
// Created by 陈鹏飞 on 2020/2/20.
//

#include "duplex-pull.h"
#include "pull-stream/include/pull.h"

namespace dp {

    void link(duplex_base *a, duplex_base *b) {
        pull::pull(a->source(), b->sink());
        pull::pull(b->source(), a->sink());
    }

    void link(const std::unique_ptr<duplex_base> &a, const std::unique_ptr<duplex_base> &b) {
        pull::pull(a->source(), b->sink());
        pull::pull(b->source(), a->sink());
    }
}
