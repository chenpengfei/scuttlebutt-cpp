//
// Created by 陈鹏飞 on 2020/1/19.
//

#include <deque>
#include <vector>
#include <iterator>
#include <functional>
#include <iostream>
#include <cstdarg>
#include "gtest/gtest.h"
#include <pull.h>
#include <values.h>
#include <map.h>

using namespace std;

TEST(combinesSourceWithThrough, pull) {

    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    auto begin = vec.begin(); //these are on the stack and are destroyed when the function ends.
    auto end = vec.end();

    auto vals = values(begin, end);

    auto mapper = [&](int val){return val * 2;};
    auto timesTwo = Map<int>(mapper);

    auto newVals = pull(vals, timesTwo);

    newVals(false, [](bool done, auto val){
        ASSERT_EQ(val, 2);
        ASSERT_FALSE(done);
    });
}
