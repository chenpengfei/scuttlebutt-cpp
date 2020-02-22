//
// Created by 陈鹏飞 on 2020/2/17.
//

#include "gtest/gtest.h"
#include "scuttlebutt.h"

using namespace std;
using namespace sb;

TEST(create_id, scuttlebutt_pull_utils) {
    auto id_1 = _create_id();
    auto id_2 = _create_id();

    ASSERT_EQ(39, id_1.length());
    ASSERT_EQ(39, id_2.length());
    ASSERT_TRUE(id_1 != id_2);
}

TEST(filter, scuttlebutt_pull_utils) {
    sb::update update_1 = make_tuple(nullptr, 2, "A", "", "");
    sb::sources sources;

    ASSERT_TRUE(_filter(update_1, sources));

    sources.insert(make_pair("A", 1));
    ASSERT_TRUE(_filter(update_1, sources));

    sources["A"] = 3;
    ASSERT_FALSE(_filter(update_1, sources));
}

TEST(sort, scuttlebutt_pull_utils) {
    sb::update update_1 = make_tuple(nullptr, 1, "A", "", "");
    sb::update update_2 = make_tuple(nullptr, 2, "B", "", "");
    sb::update update_3 = make_tuple(nullptr, 3, "C", "", "");
    sb::update update_4 = make_tuple(nullptr, 2, "A", "", "");
    sb::update update_5 = make_tuple(nullptr, 2, "C", "", "");

    std::vector<sb::update> hist{update_1, update_2, update_3, update_4, update_5};
    _sort(hist);

    ASSERT_EQ(std::get<update_items::Timestamp>(update_1), std::get<update_items::Timestamp>(hist.at(0)));
    ASSERT_EQ(std::get<update_items::SourceId>(update_1), std::get<update_items::SourceId>(hist.at(0)));

    ASSERT_EQ(std::get<update_items::Timestamp>(update_4), std::get<update_items::Timestamp>(hist.at(1)));
    ASSERT_EQ(std::get<update_items::SourceId>(update_4), std::get<update_items::SourceId>(hist.at(1)));

    ASSERT_EQ(std::get<update_items::Timestamp>(update_2), std::get<update_items::Timestamp>(hist.at(2)));
    ASSERT_EQ(std::get<update_items::SourceId>(update_2), std::get<update_items::SourceId>(hist.at(2)));

    ASSERT_EQ(std::get<update_items::Timestamp>(update_5), std::get<update_items::Timestamp>(hist.at(3)));
    ASSERT_EQ(std::get<update_items::SourceId>(update_5), std::get<update_items::SourceId>(hist.at(3)));

    ASSERT_EQ(std::get<update_items::Timestamp>(update_3), std::get<update_items::Timestamp>(hist.at(4)));
    ASSERT_EQ(std::get<update_items::SourceId>(update_3), std::get<update_items::SourceId>(hist.at(4)));
}

