#pragma once
#include <string>
#include <json.hpp>
#include <switch/types.h>

namespace common {

template<class T>
T GetEntry(const nlohmann::json& json, std::string key, T def, nlohmann::detail::value_t type) {
    auto& val = json[key];
    if (val != nullptr && val.type() == type) {
        return val.get<T>();
    }
    return def;
}

std::string GetString(const nlohmann::json& json, std::string key, std::string def);
u32 GetInt(const nlohmann::json& json, std::string key, u32 def);

nlohmann::json LoadConfig(const std::string& path, const std::string& defaultPath);

}
