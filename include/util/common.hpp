/*
 * Copyright (c) 2019-2020 ShareNX
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
#include <switch/types.h>

#include <string>

#include "json.hpp"

namespace common {

	template <class T>
	T GetEntry(const nlohmann::json &json, const char* key, T def, nlohmann::detail::value_t value_type) {
		if (json == nullptr || !json.contains(key)) {
			printf("key %s not found\n", key);
			return def;
		}
		nlohmann::json j = json[key];
		if (j.type() != value_type) {
			printf("KEY %s isn't of requested type\n", key);
			return def;
		}
		return j.get<T>();
	}

	std::string GetString(const nlohmann::json &json, const char* key, const std::string& def);
	u32 GetInt(const nlohmann::json &json, const char* key, u32 def);
	bool GetBool(const nlohmann::json &json, const char* key, bool def);

	std::pair<bool, nlohmann::json> LoadConfig(const std::string &path);

}
