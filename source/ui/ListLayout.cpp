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
#include "ui/ListLayout.hpp"

#include <switch/result.h>

#include "MainApplication.hpp"
#include "util/caps.hpp"
#include "util/host.hpp"
#include "util/set.hpp"
#include "util/theme.hpp"

extern Settings g_Settings;
extern Theme g_Theme;
extern Hoster g_Hoster;

namespace ui {

extern MainApplication *mainApp;
std::vector<CapsAlbumEntry> m_entries;

ListLayout::ListLayout() : Layout::Layout() {
	this->SetBackgroundColor(g_Theme.color.background);
	this->SetBackgroundImage(g_Theme.background_path);
	this->topRect = Rectangle::New(0, 0, 1280, 45, g_Theme.color.topbar);
	this->topText = TextBlock::New(10, 2, g_Hoster.GetName(), 35);
	this->infoText = TextBlock::New(900, 9, "\uE0E0 Select \uE0E2 Config \uE0E1 Exit", 25);
	this->topText->SetColor(g_Theme.color.text);
	this->infoText->SetColor(g_Theme.color.text);
	this->menu = FixedMenu::New(0, 45, 1280, pu::ui::Color(0, 0, 0, 0), 136, 5, 45);
	this->menu->SetOnFocusColor(g_Theme.color.focus);
	m_entries = caps::getAllEntries();
	for (auto entry : m_entries) {
		auto itm = FixedMenuItem::New(entry);
		itm->SetColor(g_Theme.color.text);
		itm->AddOnClick(std::bind(&ListLayout::onItemClick, this));
		this->menu->AddItem(itm);
	}
	this->Add(this->topRect);
	this->Add(this->topText);
	this->Add(this->infoText);
	this->Add(this->menu);
	if (!g_Theme.image.path.empty()) {
		this->image = Image::New(g_Theme.image.x, g_Theme.image.y, g_Theme.image.path);
		this->image->SetWidth(g_Theme.image.w);
		this->image->SetHeight(g_Theme.image.h);
		this->Add(this->image);
	}
}

void ListLayout::onItemClick() {
	CapsAlbumEntry entry = m_entries[this->menu->GetSelectedIndex()];
	mainApp->uploadLayout->setEntry(entry);
	mainApp->LoadLayout(mainApp->uploadLayout);
}

void ListLayout::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
	if ((Down & KEY_PLUS) || (Down & KEY_MINUS) || (Down & KEY_B)) {
		mainApp->Close();
	}

	/*if (Down & KEY_Y) {
            CapsAlbumEntry entry = m_entries[this->menu->GetSelectedIndex()];
            Result rc = caps::moveFile(entry);
            if (R_SUCCEEDED(rc)) {
                this->menu->SetSelectedIndex(0);
            } else {
                mainApp->CreateShowDialog("Failed moving file entry", "Couldn't move file with error: 0x" + scr::utl::formatResult(rc), {"OK"}, true);
            }
        }*/

	if (Down & KEY_X) {
		std::vector<std::string> hoster = g_Settings.GetHoster();
		if (hoster.size() == 0) {
			mainApp->CreateShowDialog("No site configs found", "Create your own configs and put them in /switch/screen-nx/sites/.\n\nCheck the repo for examples, or just use the default!", {"Cancel"}, true);
			return;
		}
		hoster.push_back("default");
		u64 opt = mainApp->CreateShowDialog("Select a site config", "Selecting different site configs will change the theme and\nwebsite to upload to! Selecting different\nsite configs will change the theme and website to upload to!\nSelecting different site\nconfigs will change the theme\nand website to upload to!", hoster, false);
		if (opt < 0)
			return;
		if (opt == hoster.size() - 1) {
			g_Hoster.SetDefault();
		} else {
			g_Settings.SetHoster(hoster[opt]);
		}
		mainApp->Load();
	}

	if (Down & KEY_Y) {
		std::vector<std::string> themes = g_Settings.GetThemes();
		if (themes.size() == 0) {
			mainApp->CreateShowDialog("No site configs found", "Create your own configs and put them in /switch/screen-nx/sites/.\n\nCheck the repo for examples, or just use the default!", {"Cancel"}, true);
			return;
		}
		themes.push_back("default");
		u64 opt = mainApp->CreateShowDialog("Select a site config", "Selecting different site configs will change the theme and\nwebsite to upload to! Selecting different\nsite configs will change the theme and website to upload to!\nSelecting different site\nconfigs will change the theme\nand website to upload to!", themes, false);
		if (opt < 0)
			return;
		if (opt == themes.size() - 1) {
			g_Theme.SetDefault();
		} else {
			g_Settings.SetTheme(themes[opt]);
		}
		mainApp->Load();
	}
}

} // namespace ui