#pragma once
#include <iostream>
#include <string.h>
#include "emummc_cfg.h"
#include <switch.h>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <curl/curl.h>

namespace fs = std::filesystem;
std::vector<fs::path> getDirectoryFiles(const std::string & dir, const std::vector<std::string> & extensions);
std::string getAlbumPath();
std::string uploadFile(std::string filePath);
Result smcGetEmummcConfig(emummc_mmc_t mmc_id, emummc_config_t *out_cfg, void *out_paths);
size_t WriteCallback(const char *contents, size_t size, size_t nmemb, std::string *userp);