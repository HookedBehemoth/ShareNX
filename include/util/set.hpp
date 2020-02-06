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
#include "json.hpp"
#include "util/host.hpp"
#include "util/theme.hpp"
#include <string>
#include <vector>

class Settings {
public:
	Settings();
	~Settings();
	void Initialize();
	void SetHoster(const std::string &name);
	void SetTheme(const std::string &name);
	std::vector<std::string> GetJsonEntries(const std::string &path);
	std::vector<std::string> GetHoster();
	std::vector<std::string> GetThemes();

private:
	void SetDefault();
	void createDirectory(const std::string &path);
};
