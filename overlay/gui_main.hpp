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
#include "image_item.hpp"

#include <tesla.hpp>

namespace album {

    class MainGui : public tsl::Gui {
      private:
        ImageItem *img;
        bool uploaded = false;
        CapsAlbumFileId fileId;
        u32 length;

      public:
        MainGui(const CapsAlbumFileId &fileId, const u8 *rgba_buffer, u32 video_length);
        ~MainGui();

        virtual tsl::elm::Element *createUI() override;
    };

}
