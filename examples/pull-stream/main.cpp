//
// Created by 陈鹏飞 on 2020/1/19.

#include <deque>
#include <vector>
#include <iterator>
#include <functional>
#include <iostream>
#include <cstdarg>
#include <pull.h>
#include <log.h>
#include <values.h>
#include <map.h>

using namespace std;

int main() {
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    auto begin = vec.begin(); //these are on the stack and are destroyed when the function ends.
    auto end = vec.end();

    auto vals = values(begin, end);

    auto mapper = [&](int val){return val * 2;};
    auto timesTwo = Map<int>(mapper);

    auto timesFour = pull2(timesTwo, timesTwo);
    auto newVals = pull(vals, timesFour);

    bool ended = false;
    do {
        newVals(false, [&] (bool done, int val) {
            ended = done;
            cout << val << endl;
        });
    } while(!ended);


//    auto logInt = [&] (auto read) { log<int>(read); };
//
//    pull(newVals, logInt);

    return 0;
}

