#include "util/common.hpp"
#include <fstream>

namespace common {

std::string GetString(const nlohmann::json& json, std::string key, std::string def) {
    return GetEntry<std::string>(json, key, def, nlohmann::detail::value_t::string);
}

u32 GetInt(const nlohmann::json& json, std::string key, u32 def) {
    return GetEntry<u32>(json, key, def, nlohmann::detail::value_t::number_unsigned);
}

nlohmann::json LoadConfig(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs.good()) {
        printf("Failed loading config file from %s\n", path.c_str());
        return nlohmann::json();
    }
    nlohmann::json json = nlohmann::json::parse(ifs);
    ifs.close();
    return json;
}

}