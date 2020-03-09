//
// Created by 陈鹏飞 on 2020/3/9.
//

#include "nlohmann/json.hpp"
#include "scuttlebutt-pull/scuttlebutt/include/scuttlebutt.h"

namespace std {
    using namespace sb;
    using namespace nlohmann;

    void to_json(json& j, const update& u) {
        j[update_items::Data] = std::get<update_items::Data>(u);
        j[update_items::Timestamp] = std::get<update_items::Timestamp>(u);
        j[update_items::SourceId] = std::get<update_items::SourceId>(u);
        if (!std::get<update_items::From>(u).empty()) {
            j[update_items::From] = std::get<update_items::From>(u);
        }
        if (!std::get<update_items::Singed>(u).empty()) {
            j[update_items::Singed] = std::get<update_items::Singed>(u);
        }
    }

    void from_json(const json& j, update& p) {
        std::get<update_items::Data>(p) = j.at(update_items::Data);
        std::get<update_items::Timestamp>(p) = j.at(update_items::Timestamp);
        std::get<update_items::SourceId>(p) = j.at(update_items::SourceId);

        try {
            j.at(update_items::SourceId).get_to(std::get<update_items::SourceId>(p));
        } catch (nlohmann::json::out_of_range& e) {
            spdlog::trace(e.what());
        }
        try {
            j.at(update_items::Singed).get_to(std::get<update_items::Singed>(p));
        } catch (nlohmann::json::out_of_range& e) {
            spdlog::trace(e.what());
        }
    }
} // namespace std

namespace sb {
    void to_json(nlohmann::json& j, const model_accept& m) {
        j = nlohmann::json{};
        if (!m.whitelist_.empty()) {
            j["whitelist"] = m.whitelist_;
        }
        if (!m.blacklist_.empty()) {
            j["blacklist"] = m.blacklist_;
        }
    }

    void from_json(const nlohmann::json& j, model_accept& m) {
        try {
            j.at("whitelist").get_to(m.whitelist_);
        } catch (nlohmann::json::out_of_range& e) {
            spdlog::trace(e.what());
        }
        try {
            j.at("blacklist").get_to(m.blacklist_);
        } catch (nlohmann::json::out_of_range& e) {
            spdlog::trace(e.what());
        }
    }
} // namespace sb

