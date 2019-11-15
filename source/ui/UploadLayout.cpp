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
#include "ui/UploadLayout.hpp"
#include "utils.hpp"
#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace scr::ui {
    extern MainApplication *mainApp;
    extern scr::utl::hosterConfig * m_config;

    UploadLayout::UploadLayout() : Layout::Layout() {
        this->SetBackgroundColor(COLOR(m_config->m_theme->color_background));
        this->SetBackgroundImage(m_config->m_theme->background_path);
        this->topRect = Rectangle::New(0, 0, 1280, 45, COLOR(m_config->m_theme->color_topbar));
        this->topText = TextBlock::New(10, 2, m_config->m_name, 35);
        this->infoText = TextBlock::New(1000, 9, "\uE0E0 Upload \uE0E1 Back", 25);
        this->topText->SetColor(COLOR(m_config->m_theme->color_text));
        this->infoText->SetColor(COLOR(m_config->m_theme->color_text));
        this->bottomText = TextBlock::New(70, 640, "", 45);
        this->bottomText->SetColor(COLOR(m_config->m_theme->color_text));
        this->preview = Image::New(10, 55, "");
        this->Add(this->topRect);
        this->Add(this->topText);
        this->Add(this->infoText);
        this->Add(this->bottomText);
        this->Add(this->preview);
        if (!m_config->m_theme->image_path.empty()) {
            this->image = Image::New(m_config->m_theme->image_x, m_config->m_theme->image_y, m_config->m_theme->image_path);
            this->image->SetWidth(m_config->m_theme->image_w);
            this->image->SetHeight(m_config->m_theme->image_h);
            this->Add(this->image);
        }
    }

    void UploadLayout::setEntry(scr::utl::entry * Entry) {
        m_entry = new scr::utl::entry(*Entry);
        this->m_entry = Entry;
        if (m_entry->path.find(".mp4") != std::string::npos) { // Is video
            if (m_entry->thumbnail.empty()) {
                m_entry->thumbnail = scr::utl::getThumbnail(m_entry->path.substr(5), 485, 273);
            }
            this->preview->SetImage(m_entry->thumbnail);
            this->bottomText->SetText("Upload this recording to " + m_config->m_name + "?");
        } else { // Is image
            this->preview->SetImage(m_entry->path);
            this->bottomText->SetText("Upload this screenshot to " + m_config->m_name + "?");
        }
        url = scr::utl::checkUploadCache(m_entry->path);
        if (!url.empty()) this->bottomText->SetText(url);
        this->preview->SetWidth(970);
        this->preview->SetHeight(545);
    }

    void UploadLayout::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if ((Down & KEY_PLUS) || (Down & KEY_MINUS)) {
            mainApp->Close(); 
        }

        if (Down & KEY_A) {
            if (!url.empty()) return;
            this->bottomText->SetText("Uploading... Please wait!");
            mainApp->CallForRender();
            url = scr::utl::uploadFile(m_entry->path, m_config);
            if (url.compare("")) {
                this->bottomText->SetText(url);
            } else {
                this->bottomText->SetText("Upload failed!");
            }
        }

        if (Down & KEY_B) {
            mainApp->LoadLayout(mainApp->listLayout);
        }
    }
}
