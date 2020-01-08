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
#include <switch.h>

#ifdef NXLINK_DEBUG
#define LOG(format, ...) { fprintf(stdout, "%s:%u: ", __func__, __LINE__); fprintf(stdout, format, ##__VA_ARGS__); }
#else
#define LOG(format, ...) ;
#endif

namespace fs = std::filesystem;
namespace scr::utl {
    class mimepart {
    public:
        std::string name;
        std::string data;
        bool is_file_data;
    };
    class theme {
    public:
        std::string color_text;
        std::string color_background;
        std::string color_focus;
        std::string color_topbar;
        std::string background_path;
        std::string image_path;
        u32 image_x;
        u32 image_y;
        u32 image_w;
        u32 image_h;
    };
    class hosterConfig {
    public:
        std::string m_name;
        std::string m_url;
        std::vector<mimepart> m_mimeparts;
        theme m_theme;
        hosterConfig() {}
        hosterConfig(std::string name, std::string url, std::vector<mimepart> mime, theme theme) : m_name(name), m_url(url), m_mimeparts(mime), m_theme(theme) {}
    };
    void init();
    std::string uploadFile(const std::string& path, const hosterConfig& config);
    std::vector<hosterConfig> getConfigs();
    hosterConfig getDefaultConfig();
    void setDefaultConfig(int i);
    std::string formatResult(Result rc);
}
