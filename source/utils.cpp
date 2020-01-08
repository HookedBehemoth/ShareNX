/*
 * Copyright (c) 2019 screen-nx
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils.hpp"
#include "INIReader.h"
#include <algorithm>
#include <switch.h>
#include <curl/curl.h>

#define BASE_PATH   "sdmc:/switch/screen-nx/"
#define CONFIGPATH  BASE_PATH "config.ini"
#define SITEPATH    BASE_PATH "sites/"

namespace scr::utl {
    void init() {
        if (!std::filesystem::exists(BASE_PATH))
            std::filesystem::create_directory(BASE_PATH);
        if (!std::filesystem::exists(SITEPATH))
            std::filesystem::create_directory(SITEPATH);
    }
    
    /**
     * @brief reads all configs from /config/screen-nx
     */
    std::vector<hosterConfig> getConfigs() {
        std::vector<hosterConfig> configs;
        for(fs::path file: getDirectoryFiles(SITEPATH, {".ini"})) {
            if (file.filename() == "config.ini")
                continue;
            INIReader reader(file);
            if (reader.ParseError()) {
                LOG("unable to parse %s\n", file.filename().c_str());
                continue;
            }
            hosterConfig config = parseHoster(reader);
            if (config.m_url.empty() || config.m_mimeparts.size() == 0)
                continue;
            configs.push_back(config);
        }
        return configs;
    }

    /**
     * @brief returns the default config
     */
    hosterConfig getDefaultConfig() {
        std::vector<hosterConfig> configs = getConfigs();
        int i = 0;
        LOG("parsing from %s\n", CONFIGPATH);
        INIReader reader(CONFIGPATH);
        if (!reader.ParseError()) {
            i = reader.GetInteger("screen-nx", "config_index", 0);
        } else {
            LOG("failed to parse own config\n");
            std::filesystem::remove(CONFIGPATH);
        }
        if (configs.size() > 0) {
            if (!configs[i%configs.size()].m_name.empty()) return configs[i%configs.size()];
        }
        /* load default config if everything fails */
        return hosterConfig({"lewd.pics",
                "https://lewd.pics/p/index.php",
                std::vector({
                    mimepart({"fileToUpload", "", true}),
                    mimepart({"curl", "1", false})
                }),
                theme({"#FFFFFFFF","#6C0000FF","#480001FF","#170909FF","romfs:/bg.jpg","romfs:/owo.png",975,240,292,480})});
    }

    /**
     * @brief sets the default config from the config ini.
     */
    void setDefaultConfig(int i) {
        std::filesystem::remove(CONFIGPATH);
        std::string data("[screen-nx]\nconfig_index=" + std::to_string(i));
        FILE * configFile = fopen(CONFIGPATH, "w");
        fwrite(data.c_str(), sizeof(char), data.size(), configFile);
        fflush(configFile);
        fsync(fileno(configFile));
        fclose(configFile);
    }

    std::string formatResult(Result rc) {
        char str[0x10];
        sprintf(str, "%04d-%04d", 2000 + R_MODULE(rc), R_DESCRIPTION(rc));
        return str;
    }
}
