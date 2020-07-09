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

#include <future>
#include <tesla.hpp>

namespace album {

    enum class UploadStage : u8 {
        None,
        Uploading,
        Done,
        Canceled,
    };

    class MainGui final : public tsl::Gui {
      private:
        UploadStage stage = UploadStage::None;
        const CapsAlbumFileId &fileId;
        const u8 *buffer;
        s32 img_x, img_y;
        char appId[0x11];
        char length[8];
        char date[0x20];
        float progress = 0.0f;
        std::future<std::string> response;
        std::string text;
        std::string upload;
        bool is_video;

      public:
        MainGui(const CapsAlbumFileId &fileId, const u8 *rgba_buffer, u32 video_length);
        ~MainGui();

        virtual tsl::elm::Element *createUI() override;
        virtual void update() override;
        virtual bool handleInput(u64 keysDown, u64, touchPosition touchInput, JoystickPosition, JoystickPosition) override;
    };

}
