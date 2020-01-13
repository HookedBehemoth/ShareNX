#pragma once
#include <string>
#include <json.hpp>
#include <switch/types.h>

namespace common {

template<class T>
T GetEntry(const nlohmann::json& json, std::string key, T def, nlohmann::detail::value_t value_type) {
    if (json == nullptr || !json.contains(key)) {
        printf("key %s not found\n", key.c_str());
        return def;
    }
    nlohmann::json j = json[key];
    if (j.type() != value_type) {
        printf("KEY %s isn't of requested type\n", key.c_str());
        return def;
    }
    return j.get<T>();
}

std::string GetString(const nlohmann::json& json, std::string key, std::string def);
u32 GetInt(const nlohmann::json& json, std::string key, u32 def);

std::pair<bool,nlohmann::json> LoadConfig(const std::string& path);

}
