//
// Created by 陈鹏飞 on 2020/2/20.
//

#include "model.h"

namespace sb {

    bool model::is_accepted(const model_accept &peer_accept, const update &u) {
        auto &blacklist = peer_accept.blacklist_;
        auto &whitelist = peer_accept.whitelist_;
        auto key = std::get<model_value_items::Key>(std::get<update_items::Data>(u));

        if (!blacklist.empty() && std::find(blacklist.begin(), blacklist.end(), key) != blacklist.end()) {
            return false;
        }

        if (!whitelist.empty()) {
            return (std::find(whitelist.begin(), whitelist.end(), key) != whitelist.end())? true: false;
        }

        return true;
    }

    bool model::apply_updates(const update &u) {
        auto key = std::get<model_value_items::Key>(std::get<update_items::Data>(u));

        // ignore if we already have a more recent value
        auto it = store_.find(key);
        if (it != store_.end()) {
            if (std::get<update_items::Timestamp>(it->second) > std::get<update_items::Timestamp>(u)) {
                emit("_remove", u);
                return false;
            }

            emit("_remove", *it);
        }

        store_[key] = u;

        emit("update", u);
        auto value = std::get<model_value_items::Value>(std::get<update_items::Data>(u));
        emit("changed", key, value);
        emit("changed:" + key, value);

        if (std::get<update_items::SourceId>(u) != id_) {
            emit("changedByPeer", key, value, std::get<update_items::From>(u));
        }

        return true;
    }

    std::vector<update> model::history(const sources &peer_sources, const model_accept &peer_accept) {
        std::vector<update> h;

        for (auto it = store_.begin(); it != store_.end(); ++it) {
            auto &update = it->second;
            if (!is_accepted(peer_accept, update)) {
                break;
            }
            if (_filter(update, peer_sources)) {
                h.push_back(update);
            }
        }
        _sort(h);
        return h;
    }
}
