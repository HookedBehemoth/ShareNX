#pragma once
#include <string>
#include <json.hpp>

namespace common {

template<class T>
T GetEntry(const nlohmann::json& json, std::string key, T def, nlohmann::detail::value_t type) {
    auto& val = json[key];
    if (val != nullptr && val.type() == type) {
        return val.get<T>();
    }
    return def;
}

std::string getString(const nlohmann::json& json, std::string key, std::string def) {
    return GetEntry<std::string>(json, key, def, nlohmann::detail::value_t::string);
}

u32 getInt(const nlohmann::json& json, std::string key, u32 def) {
    return GetEntry<u32>(json, key, def, nlohmann::detail::value_t::number_integer);
}

}
