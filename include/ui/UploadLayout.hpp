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

#pragma once
#include <pu/Plutonium>

using namespace pu::ui::elm;
namespace ui {
    class UploadLayout : public pu::ui::Layout
    {
        public:
            UploadLayout();
            PU_SMART_CTOR(UploadLayout)
            void setEntry(const CapsAlbumEntry& Entry);
            void onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos);
            void setProgress(double progress);
        private:
            TextBlock::Ref topText;
            TextBlock::Ref infoText;
            Rectangle::Ref topRect;
            TextBlock::Ref bottomText;
            ProgressBar::Ref progressBar;
            Image::Ref preview;
            Image::Ref image;
            std::string url;
            CapsAlbumEntry m_entry;
    };
}