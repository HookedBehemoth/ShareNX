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
    extern scr::utl::hosterConfig * m_config;
    std::vector<scr::utl::entry *> entries;

    ListLayout::ListLayout() : Layout::Layout() {
        this->SetBackgroundColor(COLOR(m_config->m_theme->color_background));
        this->SetBackgroundImage(m_config->m_theme->background_path);
        this->topRect = Rectangle::New(0, 0, 1280, 45, COLOR(m_config->m_theme->color_topbar));
        this->topText = TextBlock::New(10, 2, m_config->m_name, 35);
        this->infoText = TextBlock::New(945, 9, "(A)Select (X)Config (B)Exit", 25);
        this->topText->SetColor(COLOR(m_config->m_theme->color_text));
        this->infoText->SetColor(COLOR(m_config->m_theme->color_text));
        this->menu = FixedMenu::New(0,45,1280,COLOR("#00000000"),136,5,45);
        this->menu->SetOnFocusColor(COLOR(m_config->m_theme->color_focus));
        entries = scr::utl::getEntries();
        for (auto m_entry: entries) {
            auto itm = FixedMenuItem::New(m_entry);
            itm->SetColor(COLOR(m_config->m_theme->color_text));
            //itm->SetIcon(m_entry->small_thumbnail);
            itm->AddOnClick(std::bind(&ListLayout::onItemClick, this));
            this->menu->AddItem(itm);
        }
        this->Add(this->topRect);
        this->Add(this->topText);
        this->Add(this->infoText);
        this->Add(this->menu);
        if (!m_config->m_theme->image_path.empty()) {
            this->image = Image::New(m_config->m_theme->image_x, m_config->m_theme->image_y, m_config->m_theme->image_path);
            this->image->SetWidth(m_config->m_theme->image_w);
            this->image->SetHeight(m_config->m_theme->image_h);
            this->Add(this->image);
        }
    }

    void ListLayout::onItemClick() {
        scr::utl::entry * m_entry = entries[this->menu->GetSelectedIndex()];
        mainApp->uploadLayout->setEntry(m_entry);
        //mainApp->uploadLayout = UploadLayout::New(*m_entry);
        //mainApp->uploadLayout->SetOnInput(std::bind(&UploadLayout::onInput, mainApp->uploadLayout, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        mainApp->LoadLayout(mainApp->uploadLayout);
    }

    void ListLayout::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if ((Down & KEY_PLUS) || (Down & KEY_MINUS) || (Down & KEY_B)) {
            mainApp->Close();
        }

        if (Down & KEY_X) {
            std::vector<scr::utl::hosterConfig *> configs = scr::utl::getConfigs();
            if (configs.size() == 0) {
                mainApp->CreateShowDialog("No site configs found", "Create your own configs and put them in /switch/screen-nx/sites/.\n\nCheck the repo for examples, or just use the default!", {"Cancel"}, true);
                return;
            }
            std::vector<pu::String> options;
            for (scr::utl::hosterConfig * config: configs) options.push_back(config->m_name);
            int opt = mainApp->CreateShowDialog("Select a site config", "Selecting different site configs will change the theme and website to upload to!", options, false);
            if (opt < 0) return;
            scr::utl::setDefaultConfig(opt);
            m_config = configs[opt];
            mainApp->listLayout = ListLayout::New();
            mainApp->listLayout->SetOnInput(std::bind(&ListLayout::onInput, mainApp->listLayout, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
            mainApp->LoadLayout(mainApp->listLayout);
        }
    }
}