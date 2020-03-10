//
// Created by 陈鹏飞 on 2020/2/20.
//

#ifndef SCUTTLEBUTT_MODEL_H
#define SCUTTLEBUTT_MODEL_H

#include <iostream>
#include "scuttlebutt-pull/scuttlebutt/include/scuttlebutt.h"
#include "event-loop/include/event-loop.h"

namespace sb {

    class model final : public scuttlebutt {
    public:
        explicit model(scuttlebutt_options opts) : scuttlebutt(std::move(opts)) {

        }

        model &set(const std::string &k, const std::string &v) {

            logger->info("set: k:({}) v({})", k, v);

            local_update(std::make_pair(k, v));
            return *this;
        }

        update get_with_clock(const std::string &k) {
            auto it = store_.find(k);
            if (it != store_.end()) {
                return it->second;
            }
            return update{};
        }

        template<typename ValueType>
        ValueType get_without_clock(const std::string &k) {
            auto it = store_.find(k);
            if (it != store_.end()) {
                auto any = std::get<update_items::Data>(it->second);
                auto kv = any.get<std::pair<std::string, nlohmann::json>>();
                auto v = std::get<model_value_items::Value>(kv);
                return v.get<ValueType>();
            }
            return ValueType{};
        }

        bool is_accepted(const model_accept &peer_accept, const update &u) override;

        bool apply_updates(const update &u) override;

        std::vector<update>
        history(const sources &peer_sources, const model_accept &peer_accept) override;

    private:
        std::map<std::string, update> store_;
    };
}

#endif //SCUTTLEBUTT_MODEL_H
