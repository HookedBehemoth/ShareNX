#include "util/common.hpp"
#include "util/set.hpp"
#include "json.hpp"
#include <fstream>
#include <iomanip>
#include <filesystem>

extern Hoster g_Hoster;
extern Theme g_Theme;
Settings g_Settings;

#define BASE_PATH    "/switch/screen-nx/"
#define CONFIGPATH   BASE_PATH "config.json"
#define HOSTERPATH   BASE_PATH "hoster/"
#define THEMESPATH   BASE_PATH "themes/"

Settings::Settings() {
    createDirectory(BASE_PATH);
    createDirectory(HOSTERPATH);
    createDirectory(THEMESPATH);
    auto set = common::LoadConfig(CONFIGPATH, "romfs:/config.json");
    SetHoster(common::GetString(set, "Hoster", ""));
    SetTheme(common::GetString(set, "Theme", ""));
}

Settings::~Settings() {
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

void Settings::SetHoster(const std::string& name) {
    this->hoster = name;
    const std::string setHoster = HOSTERPATH "/" + this->hoster + ".json";
    g_Hoster = Hoster(common::LoadConfig(setHoster, "romfs:/host.json"));
}

void Settings::SetTheme(const std::string& name) {
    this->theme = name;
    const std::string setTheme = THEMESPATH "/" + this->theme + ".json";
    g_Theme = Theme(common::LoadConfig(setTheme, "romfs:/theme.json"));
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
