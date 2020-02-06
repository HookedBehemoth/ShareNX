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
#include "util/set.hpp"
#include "json.hpp"
#include "util/common.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>

Hoster g_Hoster;
Theme g_Theme;
Settings g_Settings;

#define BASE_PATH "/switch/screen-nx/"
#define CONFIGPATH BASE_PATH "config.json"
#define HOSTERPATH BASE_PATH "hoster/"
#define THEMESPATH BASE_PATH "themes/"
#define HOST "Hoster"
#define THEME "Theme"

Settings::Settings() {}

Settings::~Settings() {
	const std::string hoster = g_Hoster.name;
	const std::string theme = g_Theme.name;
	std::ofstream ofs(CONFIGPATH);
	if (!ofs.good()) {
		return;
	}
	auto cfg = nlohmann::json();
	if (!hoster.empty())
		cfg[HOST] = hoster;
	if (!theme.empty())
		cfg[THEME] = theme;
	ofs << std::setw(4) << cfg << std::endl;
	ofs.close();
}

void Settings::Initialize() {
	createDirectory(BASE_PATH);
	createDirectory(HOSTERPATH);
	createDirectory(THEMESPATH);
	const auto [loaded, json] = common::LoadConfig(CONFIGPATH);
	if (loaded) {
		auto host = common::GetString(json, "Hoster", "");
		if (!host.empty())
			this->SetHoster(host);
		auto theme = common::GetString(json, "Theme", "");
		this->SetTheme(theme);
	} else {
		std::filesystem::remove(CONFIGPATH);
		SetDefault();
	}
}

void Settings::SetHoster(const std::string &name) {
	const std::string path = HOSTERPATH + name + ".json";
	const auto [loaded, json] = common::LoadConfig(path);
	if (loaded) {
		g_Hoster.Initialize(json, name);
	} else {
		g_Hoster.SetDefault();
	}
}

void Settings::SetTheme(const std::string &name) {
	const std::string path = THEMESPATH + name + ".json";
	const auto [loaded, json] = common::LoadConfig(path);
	if (loaded) {
		g_Theme.Initialize(json, name);
	} else {
		g_Theme.SetDefault();
	}
}

void Settings::SetDefault() {
	g_Hoster.SetDefault();
	g_Theme.SetDefault();
}

std::vector<std::string> Settings::GetJsonEntries(const std::string &path) {
	std::vector<std::string> hosters;
	for (const auto &entry : std::filesystem::directory_iterator(path))
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

void Settings::createDirectory(const std::string &path) {
	if (!std::filesystem::exists(path))
		std::filesystem::create_directory(path);
}