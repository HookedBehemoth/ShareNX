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
#pragma once

#include <constants.hpp>
#include <tesla.hpp>

namespace album {

    class ImageItem : public tsl::elm::ListItem {
      private:
        CapsAlbumFileId fileId;
        const u8 *buffer;
        s32 img_x, img_y;
        char appId[0x11];
        char length[8];
        char date[0x20];
        std::string url;
        bool is_video;

      public:
        ImageItem(const CapsAlbumFileId &file_id, const u8 *image, u32 video_length);
        virtual void draw(tsl::gfx::Renderer *renderer) override;
        virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override;
        virtual tsl::elm::Element *requestFocus(tsl::elm::Element *, tsl::FocusDirection);

      public:
        void setUrl(const std::string &url) {
            this->url = url;
        }
    };

}
