//
// Created by 陈鹏飞 on 2020/3/9.
//

#include "scuttlebutt-pull/duplex/include/duplex.h"

namespace sb {

    void to_json(nlohmann::json& j, const outgoing& o) {
        j = nlohmann::json{
                {"id", o.id_},
                {"clock", o.clock_}
        };
        if (!o.accept_.empty()) {
            j["accept"] = o.accept_;
        }
    }

    void from_json(const nlohmann::json& j, outgoing& o) {
        j.at("id").get_to(o.id_);
        j.at("clock").get_to(o.clock_);
        try {
            j.at("accept").get_to(o.accept_);
        } catch (nlohmann::json::out_of_range& e) {
            spdlog::trace(e.what());
        }
    }
} // namespace sb
