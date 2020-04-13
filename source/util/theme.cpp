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
#include "util/theme.hpp"

#include <pu/Plutonium>

#include "util/common.hpp"

Theme::Theme() {}

void Theme::Initialize(const nlohmann::json &json, std::string name) {
	this->name = name;
	this->color.background = pu::ui::Color::FromHex(common::GetString(json, "Background", "#000000FF"));
	this->color.focus = pu::ui::Color::FromHex(common::GetString(json, "Focus", "#480001FF"));
	this->color.text = pu::ui::Color::FromHex(common::GetString(json, "Text", "#FFFFFFFF"));
	this->color.topbar = pu::ui::Color::FromHex(common::GetString(json, "Topbar", "#170909FF"));
	if (json.contains("Background Image"))
		this->background_path = common::GetString(json, "Background Image", "");
	else
		this->background_path = "";
	if (!json.contains("Image"))
		return;
	auto &img = json["Image"];
	if (!img.is_object())
		return;
	this->image.path = common::GetString(img, "Path", "romfs:/owo.png");
	this->image.x = common::GetInt(img, "X", 989);
	this->image.y = common::GetInt(img, "Y", 240);
	this->image.w = common::GetInt(img, "W", 291);
	this->image.h = common::GetInt(img, "H", 480);
}

void Theme::SetDefault() {
	this->name = "";
	this->color.background = pu::ui::Color(0x6c, 0x00, 0x00, 0xFF);
	this->color.focus = pu::ui::Color(0x48, 0x00, 0x01, 0xFF);
	this->color.text = pu::ui::Color(0xFF, 0xFF, 0xFF, 0xFF);
	this->color.topbar = pu::ui::Color(0x17, 0x09, 0x09, 0xFF);
	this->background_path = "romfs:/bg.jpg";
	this->image.path = "romfs:/owo.png";
	this->image.x = 989;
	this->image.y = 240;
	this->image.w = 291;
	this->image.h = 480;
}
