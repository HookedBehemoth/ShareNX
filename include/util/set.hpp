#pragma once
#include "util/host.hpp"
#include "util/theme.hpp"
#include <json.hpp>
#include <vector>
#include <string>

class Settings {
public:
    Settings();
    ~Settings();
    void SetHoster(const std::string& name);
    void SetTheme(const std::string& name);
    std::vector<std::string> GetJsonEntries(const std::string& path);
    std::vector<std::string> GetHoster();
    std::vector<std::string> GetThemes();
private:
    void createDirectory(const std::string& path);
    std::string hoster;
    std::string theme;
};
