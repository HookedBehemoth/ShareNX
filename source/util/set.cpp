#include "util/common.hpp"
#include "util/set.hpp"
#include "json.hpp"
#include <fstream>
#include <iomanip>
#include <filesystem>

Hoster g_Hoster;
Theme g_Theme;
Settings g_Settings;

#define BASE_PATH    "/switch/screen-nx/"
#define CONFIGPATH   BASE_PATH "config.json"
#define HOSTERPATH   BASE_PATH "hoster/"
#define THEMESPATH   BASE_PATH "themes/"

Settings::Settings() {}

Settings::~Settings() {
    if (this->hoster == "" && this->theme == "")
        return;
    std::ofstream ofs(CONFIGPATH);
    if (!ofs.good()) {
        return;
    }
    nlohmann::json json = {
        { "Hoster", this->hoster },
        { "Theme", this->theme }
    };
    ofs << std::setw(4) << json << std::endl;
    ofs.close();
}

void Settings::Initialize() {
    createDirectory(BASE_PATH);
    createDirectory(HOSTERPATH);
    createDirectory(THEMESPATH);
    auto json = common::LoadConfig(CONFIGPATH);
    if (json.is_object()) {
        SetHoster(common::GetString(json, "Hoster", ""));
        SetTheme(common::GetString(json, "Theme", ""));
    } else {
        std::filesystem::remove(CONFIGPATH);
        SetDefault();
    }
}

void Settings::SetHoster(const std::string& name) {
    this->hoster = name;
    const std::string setHoster = HOSTERPATH + this->hoster + ".json";
    const nlohmann::json set = common::LoadConfig(setHoster);
    g_Hoster.Initialize(set);
}

void Settings::SetTheme(const std::string& name) {
    this->theme = name;
    const std::string setTheme = THEMESPATH + this->theme + ".json";
    g_Theme.Initialize(common::LoadConfig(setTheme));
}

void Settings::SetDefault() {
    g_Hoster.Initialize(common::LoadConfig("romfs:/host.json"));
    g_Theme.Initialize(common::LoadConfig("romfs:/theme.json"));
}

std::vector<std::string> Settings::GetJsonEntries(const std::string& path) {
    std::vector<std::string> hosters;
    for (const auto & entry : std::filesystem::directory_iterator(path))
        if (entry.path().extension() == ".json")
            hosters.push_back(entry.path().stem());
    return hosters;
}

std::vector<std::string> Settings::GetHoster() {
    return GetJsonEntries(HOSTERPATH);
}

std::vector<std::string> Settings::GetThemes() {
    return GetJsonEntries(THEMESPATH);
}

void Settings::createDirectory(const std::string& path) {
    if (!std::filesystem::exists(path))
        std::filesystem::create_directory(path);
}
