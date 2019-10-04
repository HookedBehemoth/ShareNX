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

#pragma once
#include <filesystem>
#include <vector>
#include <string.h>

#ifdef NXLINK_DEBUG
#define LOG(format, ...) { fprintf(nxlinkout, "%s:%u: ", __func__, __LINE__); fprintf(nxlinkout, format, ##__VA_ARGS__); }
#else
#define LOG(format, ...) ;
#endif

namespace fs = std::filesystem;
namespace scr::utl {
    struct mimepart {
        char * name;
        char * data;
        bool is_file_data;
    };
    struct theme {
        char * color_text;
        char * color_background;
        char * color_focus;
        char * color_topbar;
        char * background_path;
        char * image_path;
        int32_t image_x;
        int32_t image_y;
        int32_t image_w;
        int32_t image_h;
    };
    struct hosterConfig {
        char * m_name;
        char * m_url;
        std::vector<mimepart *> m_mimeparts;
        theme m_theme;
    };
    struct entry {
        char * path;
        char * thumbnail;
        char * time;
    };
    std::string uploadFile(char * path, hosterConfig config);
    std::vector<hosterConfig *> getConfigs();
    hosterConfig getDefaultConfig();
    void setDefaultConfig(int i);
    std::vector<entry *> getEntries();
}