//
// Created by 陈鹏飞 on 2020/1/19.
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

TEST(combinesThroughWithThrough, pull) {
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    auto begin = vec.begin(); //these are on the stack and are destroyed when the function ends.
    auto end = vec.end();

    auto vals = values(begin, end);

    auto mapper = [&](int val) { return val * 2; };
    auto timesTwo = Map<int>(mapper);

    auto timesFour = pipe_through(timesTwo, timesTwo);
    auto newVals = pull::pull(vals, timesFour);

    newVals(dp::error::ok, [](dp::error done, auto val) {
        ASSERT_EQ(val, 4);
        ASSERT_TRUE(dp::error::ok == done);
    });

    newVals(dp::error::ok, [](dp::error done, auto val) {
        ASSERT_EQ(val, 8);
        ASSERT_TRUE(dp::error::ok == done);
    });

    newVals(dp::error::ok, [](dp::error done, auto val) {
        ASSERT_TRUE(dp::error::end == done);
    });
}