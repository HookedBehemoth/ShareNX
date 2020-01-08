#include "util/common.hpp"
#include "util/set.hpp"
#include "json.hpp"
#include <fstream>
#include <iomanip>

extern Hoster g_Hoster;
extern Theme g_Theme;

#define BASE_PATH    "/switch/screen-nx/"
#define CONFIGPATH   BASE_PATH "config.ini"
#define SITEPATH     BASE_PATH "sites/"

std::vector<FsDirectoryEntry> entries(FsFileSystem& sd_fs) {
    std::vector<FsDirectoryEntry> entries;
    FsDir site_dir;
    Result rc = fsFsOpenDirectory(&sd_fs, SITEPATH, FsDirOpenMode_ReadFiles, &site_dir);
    if (R_FAILED(rc))
        return entries;
    s64 count;
    rc = fsDirGetEntryCount(&site_dir, &count);
    if (R_FAILED(rc))
        return entries;
    s64 actual;
    entries.resize(count);
    rc = fsDirRead(&site_dir, &actual, count, entries.data());
    return entries;
}

Result GetDefaultHoster() {
    FsFileSystem sd_fs;
    Result rc = fsOpenSdCardFileSystem(&sd_fs);
    if (R_FAILED(rc))
        return rc;
}

namespace set {

Settings::Settings() {
    std::ifstream ifs(CONFIGPATH);
    if (!ifs.good()) {
        printf("FAILED TO OPEN SETTINGS FILE FOR READING");
        return;
    }
    nlohmann::json json;
    json.parse(ifs);
    g_Hoster = Hoster(json);
    ifs.close();
}

Settings::~Settings() {
    std::ofstream ofs(CONFIGPATH);
    if (!ofs.good()) {
        return;
    }
    nlohmann::json json = {
        { "Hoster", g_Hoster.GetName() },
        { "Theme", g_Theme.GetName() }
    };
    ofs << std::setw(4) << json << std::endl;
    ofs.close();
}

void Settings::SetHoster(std::string name) {
    std::ifstream ifs(name);
    if (!ifs.good())
        return;
    nlohmann::json json;
    json.parse(ifs);
    g_Hoster = Hoster(json);
    ifs.close();
}

void Settings::SetTheme(std::string name) {
    std::ifstream ifs(name);
    if (!ifs.good())
        return;
    nlohmann::json json;
    json.parse(ifs);
    g_Theme = Theme(json);
    ifs.close();
}

}