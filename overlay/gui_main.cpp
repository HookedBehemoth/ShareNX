/*
 * Copyright (c) 2020 Behemoth
 *
 * This file is part of ShareNX.
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
#include "gui_main.hpp"

#include "gui_selector.hpp"

#include <string>
#include <uploader.hpp>

namespace album {

    MainGui::MainGui(const CapsAlbumFileId &file_id, const u8 *rgba_buffer, u32 video_length)
        : fileId(file_id) {
        img = new ImageItem(file_id, rgba_buffer, video_length);
    }

    MainGui::~MainGui() {}

    tsl::elm::Element *MainGui::createUI() {
        auto rootFrame = new tsl::elm::OverlayFrame("ShareNX \uE134", VERSION);

        auto *list = new tsl::elm::List();

        list->addItem(this->img);

        auto *button = new tsl::elm::ListItem("Upload");
        button->setClickListener([&](u64 keys) {
            if (keys & KEY_A && !this->uploaded) {
                std::string url = Upload(this->fileId);
                this->uploaded = true;
                this->img->setUrl(url);
                return true;
            }
            return false;
        });
        list->addItem(button);
        auto elm = new tsl::elm::ListItem(GetHosterName());
        elm->setClickListener([](u64 keys) {
            if (keys & KEY_A) {
                tsl::changeTo<SelectorGui>();
                return true;
            }
            return false;
        });
        list->addItem(elm);

        rootFrame->setContent(list);

        return rootFrame;
    }

}
