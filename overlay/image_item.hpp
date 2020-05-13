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

#include <tesla.hpp>

constexpr size_t THUMB_WIDTH = 320, THUMB_HEIGHT = 180;
constexpr size_t THUMB_SIZE = THUMB_WIDTH * THUMB_HEIGHT * 4;
constexpr size_t WORK_SIZE = 0x10000;

class ImageItem : public tsl::elm::ListItem {
  private:
    CapsAlbumFileId fileId;
    const u8 *buffer;
    s32 img_x, img_y;
    char appId[0x11];
    char date[0x20];
    std::string url;

  public:
    ImageItem(const CapsAlbumFileId &file_id, const u8 *image)
        : ListItem(""), fileId(file_id), buffer(image), img_x(), img_y(), url() {

        std::snprintf(this->appId, 0x11, "%016lX", this->fileId.application_id);

        std::snprintf(this->date, 0x20, "%4d:%02d:%02d %02d:%02d:%02d",
                      this->fileId.datetime.year,
                      this->fileId.datetime.month,
                      this->fileId.datetime.day,
                      this->fileId.datetime.hour,
                      this->fileId.datetime.minute,
                      this->fileId.datetime.second);
    }

    virtual void draw(tsl::gfx::Renderer *renderer) override {
        u32 img_x = this->getX() + ((this->getWidth() - THUMB_WIDTH) / 2);
        renderer->drawRect(img_x, this->getY(), THUMB_WIDTH, THUMB_HEIGHT, 0xf000);
        renderer->drawBitmap(img_x, this->getY(), THUMB_WIDTH, THUMB_HEIGHT, this->buffer);

        renderer->drawString(this->appId, false, this->getX() + 15, this->getY() + THUMB_HEIGHT + 25, 20, tsl::style::color::ColorText);
        renderer->drawString(this->date, false, this->getX() + 15, this->getY() + THUMB_HEIGHT + 55, 20, tsl::style::color::ColorText);

        if (!url.empty())
            renderer->drawString(this->url.c_str(), false, this->getX() + 15, this->getY() + THUMB_HEIGHT + 85, 20, tsl::style::color::ColorText);
    }

    virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
        this->setBoundaries(this->getX(), this->getY(), this->getWidth(), THUMB_HEIGHT + 100);
    }

    virtual tsl::elm::Element *requestFocus(tsl::elm::Element *, tsl::FocusDirection) {
        return nullptr;
    }

  public:
    void setUrl(const std::string &url) {
        this->url = url;
    }
};
