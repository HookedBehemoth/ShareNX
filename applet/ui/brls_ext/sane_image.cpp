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

#include "sane_image.hpp"

#include <borealis/application.hpp>
#include <cstring>

namespace brls {

    SaneImage::~SaneImage() {
        if (this->texture != -1)
            nvgDeleteImage(Application::getNVGContext(), this->texture);
    }

    void SaneImage::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, Style *style, FrameContext *ctx) {
        nvgSave(vg);

        if (this->texture != -1) {
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x + this->imageX, y + this->imageY, this->imageWidth, this->imageHeight, this->cornerRadius);
            nvgFillPaint(vg, a(this->imgPaint));
            nvgFill(vg);
        }

        nvgRestore(vg);
    }

    void SaneImage::layout(NVGcontext *vg, Style *style, FontStash *stash) {
        nvgImageSize(vg, this->texture, &this->imageWidth, &this->imageHeight);

        const auto w = this->getWidth();
        const auto h = this->getHeight();

        this->imageX = 0;
        this->imageY = 0;

        const float viewAspectRatio  = static_cast<float>(w) / static_cast<float>(h);
        const float imageAspectRatio = static_cast<float>(this->imageWidth) / static_cast<float>(this->imageHeight);

        if (viewAspectRatio >= imageAspectRatio) {
            this->imageHeight = h;
            this->imageWidth  = this->imageHeight * imageAspectRatio;
            this->imageX      = (w - this->imageWidth) / 2.0F;
        } else {
            this->imageWidth  = w;
            this->imageHeight = this->imageWidth / imageAspectRatio;
            this->imageY      = (h - this->imageHeight) / 2.0F;
        }

        this->imgPaint = nvgImagePattern(vg, getX() + this->imageX, getY() + this->imageY, this->imageWidth, this->imageHeight, 0, this->texture, this->alpha);
    }

    View *SaneImage::getDefaultFocus() {
        return this;
    }

    void SaneImage::setImage(std::string imagePath) {
        NVGcontext *vg = Application::getNVGContext();

        if (this->texture != -1)
            nvgDeleteImage(vg, this->texture);

        this->texture = nvgCreateImage(vg, imagePath.c_str(), 0);
        this->invalidate();
    }

    void SaneImage::setImage(const unsigned char *buffer, size_t bufferSize) {
        NVGcontext *vg = Application::getNVGContext();

        if (this->texture != -1)
            nvgDeleteImage(vg, this->texture);

        this->texture = nvgCreateImageMem(vg, 0, buffer, bufferSize);
        this->invalidate();
    }

    void SaneImage::setRGBAImage(unsigned char *buffer, int width, int height) {
        NVGcontext *vg = Application::getNVGContext();

        if (this->texture != -1)
            nvgDeleteImage(vg, this->texture);

        this->texture = nvgCreateImageRGBA(vg, width, height, 0, buffer);
        this->invalidate();
    }

} // namespace brls
