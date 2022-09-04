//
// Created by IMEI on 2022/9/2.
//

#ifndef MINI_ENGINE_JSON_HPP
#define MINI_ENGINE_JSON_HPP

#include "nlohmann/json.hpp"
#include <fstream>

namespace mne {

using json = nlohmann::json;
using namespace nlohmann::json_literals;

class JsonUtils {
public:
    // 从文件中加载json
    static json load(const std::string& filename) {
        std::ifstream ifs(filename);
        json          j = json::parse(ifs);
        return j;
    }

    // 访问一条用.分割的路径
    static json& visit(json& obj, const std::string& path) {
        json*             cur = &obj;
        std::stringstream ss(path);
        std::string       item;
        try {
            while (std::getline(ss, item, '.')) {
                if (std::find_if_not(item.begin(), item.end(), [](char ch) { return std::isalnum(ch); }) == item.end()) {
                    // 全是数字
                    cur = &(cur->at(std::stoi(item)));
                } else {
                    // 字符串
                    cur = &(*cur).at(item);
                }
            }
            return *cur;
        } catch (const std::exception& err) {
            printf("JsonUtils::visit error: %s \n", err.what());
        }
    }

    // Todo 合并两个json对象
    static json merge(const json& a, const json& b) {}
};

} // namespace mne

#endif //MINI_ENGINE_JSON_HPP
