//
// Created by 陈鹏飞 on 2020/1/14.
//

#include <deque>
#include <vector>
#include <iterator>
#include "gtest/gtest.h"
#include <pull.h>
#include <values.h>
#include <map.h>

using namespace std;
using namespace pull;

TEST(returnsSourceWhenNumArgsIsOne, pull) {
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    auto begin = vec.begin(); //these are on the stack and are destroyed when the function ends.
    auto end = vec.end();

    auto vals = values(begin, end);

    auto newVals = pull::pull(vals);

    vals(false, [](bool done, auto val) {
        ASSERT_EQ(val, 1);
        ASSERT_FALSE(done);
    });
}

TEST(CanCallValues, values) {
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    auto begin = vec.begin(); //these are on the stack and are destroyed when the function ends.
    auto end = vec.end();

    ASSERT_EQ(vec.size(), 2);

    auto vals = values(begin, end);

    vals(false, [](bool done, auto val) {
        ASSERT_EQ(val, 1);
        ASSERT_FALSE(done);
    });

    vals(false, [](bool done, auto val) {
        ASSERT_EQ(val, 2);
        ASSERT_FALSE(done);
    });

    vals(false, [](bool done, auto val) {
        ASSERT_TRUE(done);
    });
}

TEST(CanMapValues, Map) {
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    auto begin = vec.begin(); //these are on the stack and are destroyed when the function ends.
    auto end = vec.end();
    auto vals = values(begin, end);
    auto mapper = [&](int val) { return val * 2; };

    auto timesTwo = Map<int>(mapper);
    auto doubled = timesTwo(vals);

    doubled(false, [](bool done, auto val) {
        ASSERT_EQ(val, 2);
        ASSERT_FALSE(done);
    });

    doubled(false, [](bool done, auto val) {
        ASSERT_EQ(val, 4);
        ASSERT_FALSE(done);
    });

    doubled(false, [](bool done, auto val) {
        ASSERT_TRUE(done);
    });
}

