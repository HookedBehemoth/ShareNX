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
#include "libffmpegthumbnailer/videothumbnailerc.h"
#include <algorithm>
#include <emummc_cfg.h>
#include <switch.h>
#include <curl/curl.h>
static const char * CONFIGPATH = "sdmc:/switch/screen-nx";
static const char * SCRCONFIGPATH = "sdmc:/switch/screen-nx/config.ini";
static const char * TEMPPATH = "sdmc:/switch/screen-nx/.temp/";

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
/*scr::utl::hosterConfig *config = (scr::utl::hosterConfig *)user;

#define SECTION(s) strcmp(section, s) == 0
#define NAME(n) strcmp(name, n) == 0

if (SECTION("hoster")) {
    if (NAME("url")) config->m_url = strdup(value);
    else if (NAME("name")) config->m_name = strdup(value);
} else if (strcmp(section, "theme") == 0) {
    if (NAME("color_text")) {
        config->m_theme->color_text = strdup(value);
    } else if (NAME("color_background")) {
        config->m_theme->color_background = strdup(value);
    } else if (NAME("color_focus")) {
        config->m_theme->color_focus = strdup(value);
    } else if (NAME("color_topbar")) {
        config->m_theme->color_topbar = strdup(value);
    } else if (NAME("background_path")) {
        config->m_theme->background_path = strdup(value);
    } else if (NAME("image_path")) {
        config->m_theme->image_path = strdup(value);
    } else if (NAME("image_x")) {
        config->m_theme->image_x = atoi(value);
    } else if (NAME("image_y")) {
        config->m_theme->image_y = atoi(value);
    } else if (NAME("image_w")) {
        config->m_theme->image_w = atoi(value);
    } else if (NAME("image_h")) {
        config->m_theme->image_h = atoi(value);
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
        config->m_mimeparts[index]->is_file_data = (strcmp(value, "true") == 0) ? true: false;
    } else {
        return 0;
    }
}*/
return 1;
}

static int config_handler(void* user, const char* section, const char* name, const char* value) {
    #define SECTION(s) strcmp(section, s) == 0
    #define NAME(n) strcmp(name, n) == 0
    int * userval = (int *)user;
    if(SECTION("screen-nx")) {
        if(NAME("config_index")) {
            *userval = atoi(value);
        }
    }
    return 0;
}

namespace scr::utl {
    void init() {
        if (!std::filesystem::exists(CONFIGPATH))
            std::filesystem::create_directory(CONFIGPATH);
        if (!std::filesystem::exists(TEMPPATH))
            std::filesystem::create_directory(TEMPPATH);
    }

    void clearCacheMonthly() {
        for(fs::path file: getDirectoryFiles(TEMPPATH, {".txt",".png"})) {
            std::time_t currentTime = std::time(nullptr);
            std::time_t lastWriteTimePlusAMonth = std::chrono::system_clock::to_time_t(std::filesystem::last_write_time(file)) + 2592000;
            if (currentTime > lastWriteTimePlusAMonth) remove(file);
        }
    }

    /**
     * @brief Check if file has been uploaded previously
     * @param path to the file to upload.
     * @return url to the upload
     */
    std::string checkUploadCache(std::string path) {
        std::string txtloc = TEMPPATH + fs::path(path).filename().string() + ".txt";
        if(std::filesystem::exists(txtloc)) {
            std::time_t currentTime = std::time(nullptr);
            std::time_t lastWriteTimePlusADay = std::chrono::system_clock::to_time_t(std::filesystem::last_write_time(txtloc)) + 86400;
            if (currentTime < lastWriteTimePlusADay) {
                FILE * file = fopen(txtloc.c_str(), "r");
                char line[1024];
                fgets(line, 1024, file);
                std::string url = line;
                fflush(file);
                fclose(file);
                return url;
            }
        }
        return "";
    }

    /**
     * @brief Store a URL for an uploaded file
     * @param path to the file uploaded.
     * @param url to the file uploaded.
     */
    void storeCachedURL(std::string path, std::string url) {
        std::string cachedURLFile = TEMPPATH + fs::path(path).filename().string() + ".txt";
        FILE * file = fopen(cachedURLFile.c_str(), "w");
        fwrite(url.c_str(), sizeof(char), url.size(), file);
        fflush(file);
        fclose(file);
    }

    /**
     * @brief Uploads a file to a hoster.
     * @param path Path to the file to upload.
     * @param config HosterConfig to upload to.
     * @return url to the upload
     */
    std::string uploadFile(std::string path, hosterConfig * config) {
        std::string cachedURL = checkUploadCache(path);
        if (!cachedURL.empty()) return cachedURL;

        CURL * curl = curl_easy_init();
        curl_mime * mime;
        mime = curl_mime_init(curl);
        
        for (mimepart * m_mimepart : *config->m_mimeparts) {
            curl_mimepart * cmimepart = curl_mime_addpart(mime);
            curl_mime_name(cmimepart, m_mimepart->name.c_str());
            if (m_mimepart->is_file_data) {
                curl_mime_filedata(cmimepart, path.c_str());
            } else {
                curl_mime_data(cmimepart, m_mimepart->data.c_str(), CURL_ZERO_TERMINATED);
            }
        }

        std::string * urlresponse = new std::string;
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)urlresponse);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        curl_easy_setopt(curl, CURLOPT_URL, config->m_url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            LOG("perform failed with %d\n", res);
        }

        int rcode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rcode);
        
        LOG("response code: %d\n", rcode)
        LOG("urlresponse: %s\n", urlresponse->c_str())

        curl_easy_cleanup(curl);
        curl_mime_free(mime);

        storeCachedURL(path, *urlresponse);

        return *urlresponse;        
    }

    static void parseHoster(INIReader reader, hosterConfig * config) {
        /* hoster */
        config->m_name = reader.GetString("hoster", "name", "");
        config->m_url = reader.GetString("hoster", "url", "");
        /* mime */
        config->m_mimeparts = new std::vector<mimepart *>();
        for (int i = 0; i < reader.GetInteger("hoster", "mime_count", 0); i++) {
            mimepart * part = new mimepart();
            part->name = reader.GetString(std::to_string(i), "name", "");
            part->data = reader.GetString(std::to_string(i), "data", "");
            part->is_file_data = reader.GetBoolean(std::to_string(i), "is_file_data", false);
            config->m_mimeparts->push_back(part);
        }
        /* theme */
        #define THEMESTRING(name, def) reader.GetString("theme", name, def)
        #define THEMEINT(name) reader.GetInteger("theme", name, 0)
        config->m_theme = new theme();
        config->m_theme->color_text = THEMESTRING("color_text", "#FFFFFFFF");
        config->m_theme->color_background = THEMESTRING("color_background", "#6C0000FF");
        config->m_theme->color_focus = THEMESTRING("color_focus", "#480001FF");
        config->m_theme->color_topbar = THEMESTRING("color_topbar", "#170909FF");
        config->m_theme->background_path = THEMESTRING("background_path", "");
        config->m_theme->image_path = THEMESTRING("image_path", "");
        config->m_theme->image_x = THEMEINT("image_x");
        config->m_theme->image_y = THEMEINT("image_y");
        config->m_theme->image_w = THEMEINT("image_w");
        config->m_theme->image_h = THEMEINT("image_h");

        LOG("parsed ini file: %s\n", config->m_name.c_str()); 
    }
    
    /**
     * @brief reads all configs from /config/screen-nx
     */
    std::vector<hosterConfig *> getConfigs() {
        std::vector<hosterConfig *> * vector = new std::vector<hosterConfig *>;
        for(fs::path file: getDirectoryFiles(CONFIGPATH, {".ini"})) {
            if (file.filename() == "config.ini") continue;
            hosterConfig * config = new hosterConfig;
            INIReader reader(file);
            if (reader.ParseError()) {
                LOG("unable to parse %s\n", file.filename().c_str());
                continue;
            }
            parseHoster(reader, config);
            if (config->m_url.empty() || config->m_mimeparts->size() == 0)
                continue;
            vector->push_back(config);
        }
        return *vector;
    }

    /**sdmc:/config/screen-nx/config.ini
     * @brief returns the default config
     */
    hosterConfig * getDefaultConfig() {
        std::vector<hosterConfig *> configs = getConfigs();
        int i = 0;
        LOG("parsing from %s\n", SCRCONFIGPATH);
        INIReader reader(SCRCONFIGPATH);
        if (!reader.ParseError()) {
            i = reader.GetInteger("screen-nx", "config_index", 0);
        } else {
            LOG("failed to parse own config\n");
        }
        if (configs.size() > 0) {
            if (!configs[i%configs.size()]->m_name.empty()) return configs[i%configs.size()];
        }
        /* load default config if everything fails */
        return new hosterConfig({"lewd.pics",
                "https://lewd.pics/p/index.php",
                new std::vector({
                    new mimepart({"fileToUpload", "", true}),
                    new mimepart({"curl", "1", false})
                }),
                new theme({"#FFFFFFFF","#6C0000FF","#480001FF","#170909FF","romfs:/bg.png","romfs:/owo.png",989,240,292,480})});
    }

    /**
     * @brief sets the default config from the config ini.
     */
    void setDefaultConfig(int i) {
        std::string data("[screen-nx]\nconfig_index=" + std::to_string(i));
        FILE * configFile = fopen(SCRCONFIGPATH, "w");
        fwrite(data.c_str(), sizeof(char), data.size(), configFile);
        fflush(configFile);
        fsync(fileno(configFile));
        fclose(configFile);
    }

    void callback(ThumbnailerLogLevel lvl, const char* msg) {
        LOG("getThumbnail: %s: %s\n", (lvl == 0)? "warning": "error", msg);
    }

    std::string getThumbnail(std::string file, int width, int height) {
        std::string thumbpath = TEMPPATH + fs::path(file).filename().string().append(std::to_string(width)).append("x").append(std::to_string(height)).append(".png");
        if (std::filesystem::exists(thumbpath)) return thumbpath;
        if (fs::path(file).extension() == ".jpg") {
            // TODO: hf huntereb
            return file;
        } else if (fs::path(file).extension() == ".png") {
            // TODO: hf huntereb
            return file;
        } else if (fs::path(file).extension() == ".mp4") {
            video_thumbnailer * vth = video_thumbnailer_create();
            video_thumbnailer_set_log_callback(vth, callback);
            video_thumbnailer_set_size(vth, width, height);
            vth->overlay_film_strip = 1;
            int rc = video_thumbnailer_generate_thumbnail_to_file(vth, file.c_str(), thumbpath.c_str());
            video_thumbnailer_destroy(vth);
            LOG("thumbnail rc=%d\n", rc);
            if (rc == 0 ) return thumbpath;
            else return "romfs:/video.png";
        } else {
            return file;
        }
    }

    /**
     * @brief get's all file entries for listing.
     */
    std::vector<entry *> getEntries() {
        std::vector<entry *> * entries = new std::vector<entry *>;
        for (auto& file: getDirectoryFiles("sdmc:/" + getAlbumPath(), {".jpg", ".png", ".mp4"})) {
            LOG("added %s\n", file.filename().c_str());
            entry * m_entry = new entry;
            m_entry->path = file;
            m_entry->title = file.filename().string().substr(0,12).insert(10,":").insert(8," ").insert(6,".").insert(4,".");
            entries->push_back(m_entry);
            m_entry->small_thumbnail = getThumbnail(file.string().substr(5), 224, 136);
        }
        return *entries;
    }
}
