/*
    Borealis, a Nintendo Switch UI Library
    Copyright (C) 2019  WerWolv
    Copyright (C) 2019  p-sam

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <borealis/frame_context.hpp>
#include <borealis/view.hpp>

namespace brls {

    // A sane image
    class SaneImage : public View {
      public:
        SaneImage() = default;
        ~SaneImage();

        SaneImage(const SaneImage &)  = delete;
        SaneImage(const SaneImage &&) = delete;
        SaneImage &operator=(const SaneImage &) = delete;
        SaneImage &operator=(const SaneImage &&) = delete;

        virtual void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, Style *style, FrameContext *ctx) override;
        virtual void layout(NVGcontext *vg, Style *style, FontStash *stash) override;
        virtual View *getDefaultFocus() override;

        void setCornerRadius(float radius) {
            this->cornerRadius = radius;
        }

        void setImage(std::string imagePath);
        void setImage(unsigned char *buffer, size_t bufferSize);
        void setRGBAImage(unsigned char *buffer, int width, int height);

      private:
        int texture = -1;
        NVGpaint imgPaint;

        float cornerRadius = 0;

        int imageX = 0, imageY = 0;
        int imageWidth = 0, imageHeight = 0;
    };

} // namespace brls
