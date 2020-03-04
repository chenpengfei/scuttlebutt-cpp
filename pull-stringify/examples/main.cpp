//
// Created by 陈鹏飞 on 2020/1/19.

#include "pull-stringify.h"
#include "pull-stream/include/values.h"
#include "pull-stream/include/pull.h"
#include "pull-stream/include/log.h"

using namespace std;

// a simple struct to model a person
struct person {
    std::string name;
    std::string address;
    int age;
};

void to_json(nlohmann::json& j, const person& p) {
    j = nlohmann::json{{"name", p.name}, {"address", p.address}, {"age", p.age}};
}

void from_json(const nlohmann::json& j, person& p) {
    j.at("name").get_to(p.name);
    j.at("address").get_to(p.address);
    j.at("age").get_to(p.age);
}

int main() {
    int n = 5;
    vector<person> vec;
    vec.reserve(n);
    for (auto i = 1; i < n; ++i) {
        vec.push_back(person{to_string(i)+"_name", to_string(i)+"address", i});
    }

    auto begin = vec.begin();
    auto end = vec.end();
    auto source = pull::values(begin, end);
    auto sink = [&](auto read) { pull::log_with_looper<nlohmann::json>(read); };

    pull_stringify through;
    pull::pull(source, through.serialize(), sink);

    return 0;
}

