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

#include "MainApplication.hpp"
#include "ui/ListLayout.hpp"
#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace scr::ui {
    extern MainApplication *mainApp;
    extern scr::utl::hosterConfig m_config;
    std::vector<scr::utl::entry *> entries;

    ListLayout::ListLayout() : Layout::Layout() {
        this->SetBackgroundColor(COLOR(m_config.m_theme.color_background));
        this->SetBackgroundImage(m_config.m_theme.background_path);
        this->topRect = Rectangle::New(0, 0, 1280, 30, COLOR(m_config.m_theme.color_topbar));
        this->topText = TextBlock::New(10, 0, m_config.m_name, 25);
        this->topText->SetColor(COLOR(m_config.m_theme.color_text));
        if (!strcmp(m_config.m_theme.image_path, "") == 0) {
            this->image = Image::New(m_config.m_theme.image_x, m_config.m_theme.image_y, m_config.m_theme.image_path);
            this->image->SetWidth(m_config.m_theme.image_w);
            this->image->SetHeight(m_config.m_theme.image_h);
            this->Add(this->image);
        }
        this->menu = FixedMenu::New(0,40,980,COLOR(m_config.m_theme.color_background),136,5,45);
        this->menu->SetOnFocusColor(COLOR(m_config.m_theme.color_focus));
        entries = scr::utl::getEntries();
        for (auto m_entry: scr::utl::getEntries()) {
            auto itm = FixedMenuItem::New(m_entry->time);
            itm->SetColor(COLOR(m_config.m_theme.color_text));
            itm->SetIcon(m_entry->thumbnail);
            itm->AddOnClick(std::bind(&ListLayout::onItemClick, this));
            this->menu->AddItem(itm);
        }
        this->Add(this->topRect);
        this->Add(this->topText);
        this->Add(this->menu);
    }

    void ListLayout::onItemClick() {
        scr::utl::entry * m_entry = entries[this->menu->GetSelectedIndex()];
        mainApp->uploadLayout = UploadLayout::New(*m_entry);
        mainApp->uploadLayout->SetOnInput(std::bind(&UploadLayout::onInput, mainApp->uploadLayout, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        mainApp->LoadLayout(mainApp->uploadLayout);
    }

    void ListLayout::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if ((Down & KEY_PLUS) || (Down & KEY_MINUS) || (Down & KEY_B)) {
            mainApp->Close();
        }

        if (Down & KEY_X) {
            std::vector<scr::utl::hosterConfig *> configs = scr::utl::getConfigs();
            std::vector<pu::String> options;
            for (scr::utl::hosterConfig * config: configs) options.push_back(config->m_name);
            int opt = mainApp->CreateShowDialog("Select config", "select a config", options, false);
            if(opt < 0)
            {
                return;
            }
            scr::utl::setDefaultConfig(opt);
            m_config = *configs[opt];
            mainApp->listLayout = ListLayout::New();
            mainApp->listLayout->SetOnInput(std::bind(&ListLayout::onInput, mainApp->listLayout, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
            mainApp->LoadLayout(mainApp->listLayout);
        }
    }
}