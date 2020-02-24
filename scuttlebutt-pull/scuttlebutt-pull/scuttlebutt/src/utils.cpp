//
// Created by 陈鹏飞 on 2020/2/16.
//

#include "scuttlebutt.h"

namespace sb {

    std::string _create_id() {
        char hex_characters[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
                                 'D',
                                 'E', 'F'};

        std::string id;
        for (auto i = 0; i < 39; ++i) {
            id.append(1, hex_characters[rand() % 16]);
        }
        return id;
    }

    bool _filter(const sb::update &update, const sb::sources &sources) {
        // update in local store
        auto ts = std::get<sb::update_items::Timestamp>(update);
        auto source_id = std::get<sb::update_items::SourceId>(update);

        auto it = sources.find(source_id);
        return it == sources.end() ? true : it->second < ts;
    }

    void _sort(std::vector<sb::update> &hist) {
        std::sort(hist.begin(), hist.end(), [](const sb::update &a, const sb::update &b) {
            // sort by timestamps, then ids.
            // there should never be a pair with equal timestamps
            // and ids.
            auto ta = std::get<sb::update_items::Timestamp>(a);
            auto tb = std::get<sb::update_items::Timestamp>(b);
            return ta != tb ? ta < tb : std::get<sb::update_items::SourceId>(a) <
                                        std::get<sb::update_items::SourceId>(b);
        });
    }

}
