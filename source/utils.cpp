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
#include "ini.h"
#include <algorithm>
#include <emummc_cfg.h>
#include <switch.h>
#include <curl/curl.h>
#define CONFIGPATH "sdmc:/config/screen-nx"
#define SCRCONFIGPATH "sdmc:/config/screen-nx/config.ini"

std::vector<fs::path> getDirectoryFiles(const std::string & dir, const std::vector<std::string> & extensions) {
    std::vector<fs::path> files;
    for(auto & p: fs::recursive_directory_iterator(dir))
    {
        if (fs::is_regular_file(p))
        {
                if (extensions.empty() || std::find(extensions.begin(), extensions.end(), p.path().extension().string()) != extensions.end())
            {
                files.push_back(p.path());
            }
        }
    }
    std::sort(files.begin(), files.end());
    std::reverse(files.begin(), files.end());
    return files;
}

Result smcGetEmummcConfig(emummc_mmc_t mmc_id, emummc_config_t *out_cfg, void *out_paths) {
    SecmonArgs args;
    args.X[0] = 0xF0000404;
    args.X[1] = mmc_id;
    args.X[2] = (u64)out_paths;
    Result rc = svcCallSecureMonitor(&args);
    if (rc == 0)
    {
        if (args.X[0] != 0)
        {
            rc = (26u | ((u32)args.X[0] << 9u));
        }
        if (rc == 0)
        {
            memcpy(out_cfg, &args.X[1], sizeof(*out_cfg));
        }
    }
    return rc;
}

std::string getAlbumPath() {
    std::string out = "Nintendo/Album";
    static struct
    {
        char storage_path[0x7F + 1];
        char nintendo_path[0x7F + 1];
    } __attribute__((aligned(0x1000))) paths;

    emummc_config_t config;

    int x = smcGetEmummcConfig(EMUMMC_MMC_NAND, &config, &paths);
    if(x != 0) return out;
    if(config.base_cfg.type == 0) return out;
    out = paths.nintendo_path;
    out += "/Album";
    return out;
}

size_t CurlWriteCallback(const char *contents, size_t size, size_t nmemb, std::string *userp) {
	userp->append(contents, size * nmemb);
	return size * nmemb;
}

static int hoster_handler(void* user, const char* section, const char* name, const char* value) {
    scr::utl::hosterConfig *config = (scr::utl::hosterConfig *)user;

    #define SECTION(s) strcmp(section, s) == 0
    #define NAME(n) strcmp(name, n) == 0

    if (SECTION("hoster")) {
        if (NAME("url")) config->m_url = strdup(value);
        else if (NAME("name")) config->m_name = strdup(value);
    } else if (strcmp(section, "theme") == 0) {
        if (NAME("color_text")) {
            config->m_theme.color_text = strdup(value);
        } else if (NAME("color_background")) {
            config->m_theme.color_background = strdup(value);
        } else if (NAME("color_focus")) {
            config->m_theme.color_focus = strdup(value);
        } else if (NAME("color_topbar")) {
            config->m_theme.color_topbar = strdup(value);
        } else if (NAME("background_path")) {
            config->m_theme.background_path = strdup(value);
        } else if (NAME("image_path")) {
            config->m_theme.image_path = strdup(value);
        } else if (NAME("image_x")) {
            config->m_theme.image_x = atoi(value);
        } else if (NAME("image_y")) {
            config->m_theme.image_y = atoi(value);
        } else if (NAME("image_w")) {
            config->m_theme.image_w = atoi(value);
        } else if (NAME("image_h")) {
            config->m_theme.image_h = atoi(value);
        }
    } else {
        int index;
        try {
            index = atoi(section);
        } catch (std::exception& e) {
            LOG("An error occurred:\n%s", e.what());
            return 0;
        }
		while (config->m_mimeparts.size() < index+1) {
            config->m_mimeparts.push_back(new scr::utl::mimepart);
        }
        if (NAME("name")) {
            config->m_mimeparts[index]->name = strdup(value);
        } else if (NAME("data")) {
            config->m_mimeparts[index]->data = strdup(value);
        } else if (NAME("is_file_data")) {
            config->m_mimeparts[index]->is_file_data = strcmp(value, "true") == 0 ? true: false;
        } else {
			return 0;
		}
    }
    return 1;
}

static int config_handler(void* user, const char* section, const char* name, const char* value) {
    if(strcmp(section, "screen-nx") == 0 && strcmp(name, "config_index") == 0)
        *((int *)user) = atoi(value);
    return 0;
}

namespace scr::utl {
    /**
     * @brief Uploads a file to a hoster.
     * @param path Path to the file to upload.
     * @param config HosterConfig to upload to.
     * @return url to the upload
     */
    std::string uploadFile(char * path, hosterConfig config) {
        CURL * curl = curl_easy_init();
        curl_mime * mime;
        mime = curl_mime_init(curl);
        
        for (mimepart * m_mimepart : config.m_mimeparts) {
            curl_mimepart * cmimepart = curl_mime_addpart(mime);
            curl_mime_name(cmimepart, m_mimepart->name);
            if (m_mimepart->is_file_data) {
                curl_mime_filedata(cmimepart, path);
            } else {
                curl_mime_data(cmimepart, m_mimepart->data, CURL_ZERO_TERMINATED);
            }
        }

        std::string * urlresponse = new std::string;
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)urlresponse);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        curl_easy_setopt(curl, CURLOPT_URL, config.m_url);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            LOG("perform failed with %s\n", res);
        }

        int rcode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rcode);
        
        LOG("response code: %d\n", rcode)
        LOG("urlresponse: %s\n", urlresponse->c_str())

        curl_easy_cleanup(curl);
        curl_mime_free(mime);

        return *urlresponse;        
    }
    
    /**
     * @brief reads all configs from /config/screen-nx
     */
    std::vector<hosterConfig *> getConfigs() {
        std::vector<hosterConfig *> * vector = new std::vector<hosterConfig *>;
        for(fs::path file: getDirectoryFiles(CONFIGPATH, {".ini"})) {
            if (file.filename() == "config.ini") continue;
            hosterConfig * config = new hosterConfig;
            if (ini_parse(file.c_str(), hoster_handler, config) < 0) {
                LOG("Couldn't parse %s\n", file.c_str())
                continue;
            }
            vector->push_back(config);
        }
        return *vector;
    }

    /**
     * @brief returns the default config
     */
    hosterConfig getDefaultConfig() {
        std::vector<hosterConfig *> configs = getConfigs();
        int i = 0;
        if (ini_parse(SCRCONFIGPATH, config_handler, &i) < 0) {
            LOG("failed to parse own config\n")
        }
        if (configs.size() > 0) {
            return *configs[i%configs.size()];
        }
        /* load default config if everything fails */
        return {"lewd.pics",
                "https://lewd.pics/p/index.php",
                { new mimepart({"fileToUpload", "", true}), new mimepart({"curl", "1", false})},
                {"#FFFFFFFF","#6c0000FF","#480001FF","#170909FF","romfs:/bg.png","romfs:/owo.png",989,240,291,480}};
    }

    /**
     * @brief sets the default config from the config ini.
     */
    void setDefaultConfig(int i) {
    }

    /**
     * @brief get's all file entries for listing.
     */
    std::vector<entry *> getEntries() {
        std::vector<entry *> * entries = new std::vector<entry *>;
        for (auto& file: getDirectoryFiles("sdmc:/" + getAlbumPath(), {".jpg", ".png", ".mp4"})) {
            printf("%s\n", file.filename().c_str());
            entry * m_entry = new entry;
            m_entry->path = strdup(file.c_str());
            m_entry->thumbnail = "";
            m_entry->time = strdup(file.filename().string().substr(0,12).insert(10,":").insert(8," ").insert(6,".").insert(4,".").c_str());
            entries->push_back(m_entry);
            if (file.filename().extension() == ".mp4") m_entry->thumbnail = "romfs:/video.png";
            else m_entry->thumbnail = m_entry->path;
        }
        return *entries;
    }
}