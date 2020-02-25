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
#include <switch.h>

#include "json.hpp"
#include "ui/UploadLayout.hpp"

struct Mime {
	std::string name, data;
};

class Hoster {
public:
	Hoster();
	~Hoster();
	void Initialize(const nlohmann::json &json, std::string name);
	void SetDefault();
	std::string GetName();
	std::string GetUrl();
	std::string GetRegex();
	std::string uploadEntry(const CapsAlbumEntry &entry, ui::UploadLayout *cb_data);
	std::string name;
private:
	std::string url;
	std::string imageMimeName;
	std::string videoMimeName;
	std::string regex;
	std::vector<Mime> mimeData;
};
