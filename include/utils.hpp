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
#include <string.h>
#include "emummc_cfg.h"
#include <switch.h>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <curl/curl.h>
#include <pu/Plutonium>
#include <ctime>

namespace fs = std::filesystem;
std::vector<fs::path> getDirectoryFiles(const std::string & dir, const std::vector<std::string> & extensions);
std::string getAlbumPath();
std::string getUrl(fs::path path);
std::string uploadFile(std::string filePath);
Result smcGetEmummcConfig(emummc_mmc_t mmc_id, emummc_config_t *out_cfg, void *out_paths);
size_t WriteCallback(const char *contents, size_t size, size_t nmemb, std::string *userp);
struct appcolor {
    pu::ui::Color WHITE, RED, DRED, TOPBAR;
};
