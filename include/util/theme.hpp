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
#include <pu/ui/ui_Types.hpp>

#include "json.hpp"

class Theme {
public:
	Theme();
	void Initialize(const nlohmann::json &json, std::string name);
	void SetDefault();
	std::string name;
	struct {
		pu::ui::Color background;
		pu::ui::Color focus;
		pu::ui::Color text;
		pu::ui::Color topbar;
	} color;
	std::string background_path;
	struct {
		std::string path;
		u32 x, y, w, h;
	} image;
};
