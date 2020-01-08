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
#include "util/caps.hpp"
#include "util/host.hpp"
#include "util/theme.hpp"
#define COLOR(hex) pu::ui::Color::FromHex(hex)

extern Theme g_Theme;
extern Hoster g_Hoster;

namespace ui {
    extern MainApplication *mainApp;

    UploadLayout::UploadLayout() : Layout::Layout() {
        this->SetBackgroundColor(g_Theme.color.background);
        this->SetBackgroundImage(g_Theme.background_path);
        this->topRect = Rectangle::New(0, 0, 1280, 45, g_Theme.color.topbar);
        this->topText = TextBlock::New(10, 2, g_Hoster.GetName(), 35);
        this->infoText = TextBlock::New(1000, 9, "\uE0E0 Upload \uE0E1 Back", 25);
        this->topText->SetColor(g_Theme.color.text);
        this->infoText->SetColor(g_Theme.color.text);
        this->bottomText = TextBlock::New(70, 640, "", 45);
        this->bottomText->SetColor(g_Theme.color.text);
        this->preview = MImage::New(10, 55, "");
        this->Add(this->topRect);
        this->Add(this->topText);
        this->Add(this->infoText);
        this->Add(this->bottomText);
        this->Add(this->preview);
        if (!g_Theme.image.path.empty()) {
            this->image = Image::New(g_Theme.image.x, g_Theme.image.y, g_Theme.image.path);
            this->image->SetWidth(g_Theme.image.w);
            this->image->SetHeight(g_Theme.image.h);
            this->Add(this->image);
        }
    }

    void UploadLayout::setEntry(const CapsAlbumEntry& entry) {
        this->m_entry = entry;
        u64 img_size = 1280*720*4;
        u64 w, h;
        void* buffer = malloc(img_size);
        Result rc = caps::getImage(&w, &h, entry, buffer, img_size);
        if (R_SUCCEEDED(rc)) {
            this->preview->SetRawImage(buffer, w, h);
        }
        free(buffer);
        /*u64 image_size = entry.v1.size;
        void* buffer = malloc(image_size);
        Result rc = caps::getFile(this->m_entry, CapsAlbumStorage_Sd, buffer);
        if (R_SUCCEEDED(rc)) {
            this->preview->SetImage(buffer, image_size);
        }
        free(buffer);*/
        this->bottomText->SetText("Upload this screenshot to " + g_Hoster.GetName() + "?");
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
            //url = scr::utl::uploadFile(m_entry->path, m_config);
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
