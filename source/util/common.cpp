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
#include "util/common.hpp"

#include <fstream>

namespace common {

	std::string GetString(const nlohmann::json &json, std::string key, std::string def) {
		return GetEntry<std::string>(json, key, def, nlohmann::detail::value_t::string);
	}

	u32 GetInt(const nlohmann::json &json, std::string key, u32 def) {
		return GetEntry<u32>(json, key, def, nlohmann::detail::value_t::number_unsigned);
	}

	std::pair<bool, nlohmann::json> LoadConfig(const std::string &path) {
		std::ifstream ifs(path);
		if (!ifs.good()) {
			printf("Failed loading config file from %s\n", path.c_str());
			return std::make_pair(false, nlohmann::json());
		}
		nlohmann::json json = nlohmann::json::parse(ifs);
		ifs.close();
		if (json == nullptr || !json.is_object()) {
			printf("Parsing config file %s failed\n", path.c_str());
			return std::make_pair(false, nlohmann::json());
		}
		return std::make_pair(true, json);
	}

}
