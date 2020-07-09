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

#include <constants.hpp>
#include <string>
#include <uploader.hpp>

using namespace std::chrono_literals;
using namespace std::string_literals;

namespace album {

    MainGui::MainGui(const CapsAlbumFileId &file_id, const u8 *rgba_buffer, u32 video_length) : fileId(file_id), buffer(rgba_buffer) {
        auto uploader = GetDefaultHoster();
        this->upload  = "Upload to "s + uploader.name;

        is_video = file_id.content == CapsAlbumFileContents_Movie;

        u8 seconds = (video_length + 999) / 1000;

        std::snprintf(this->length, 8, "%dsec", seconds);

        std::snprintf(this->appId, 0x11, "%016lX", this->fileId.application_id);

        std::snprintf(this->date, 0x20, "%4d:%02d:%02d %02d:%02d:%02d",
                      this->fileId.datetime.year,
                      this->fileId.datetime.month,
                      this->fileId.datetime.day,
                      this->fileId.datetime.hour,
                      this->fileId.datetime.minute,
                      this->fileId.datetime.second);
    }

    MainGui::~MainGui() {}

    tsl::elm::Element *MainGui::createUI() {
        auto rootFrame = new tsl::elm::OverlayFrame("ShareNX \uE134", VERSION);

        auto drawer = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            u32 img_x = x + ((w - ThumbnailWidth) / 2);
            renderer->drawRect(img_x, y, ThumbnailWidth, ThumbnailHeight, 0xf000);
            renderer->drawBitmap(img_x, y, ThumbnailWidth, ThumbnailHeight, this->buffer);

            if (is_video) {
                u32 vw = (ThumbnailWidth / 4);
                u32 vh = (ThumbnailHeight / 6);
                u32 vx = img_x + ThumbnailWidth - vw;
                u32 vy = y + ThumbnailHeight - vh;
                renderer->drawRect(vx, vy, vw, vh, tsl::Color(0, 0, 0, 0xa));
                renderer->drawString(this->length, false, vx + 10, vy + vh - 4, 22, tsl::style::color::ColorText);
            }

            renderer->drawString(this->appId, false, x + 15, y + ThumbnailHeight + 25, 20, tsl::style::color::ColorText);
            renderer->drawString(this->date, false, x + 15, y + ThumbnailHeight + 55, 20, tsl::style::color::ColorText);

            switch (this->stage) {
                case UploadStage::None: {
                    /* Upload button */
                    static float counter      = 0;
                    const float progress      = (std::sin(counter) + 1) / 2;
                    tsl::Color highlightColor = {static_cast<u8>((0x2 - 0x8) * progress + 0x8),
                                                 static_cast<u8>((0x8 - 0xF) * progress + 0xF),
                                                 static_cast<u8>((0xC - 0xF) * progress + 0xF),
                                                 0xF};
                    counter += 0.1F;

                    auto hx = x + 10;
                    auto hy = y + 350;
                    auto hw = w - 20;
                    auto hh = 70;
                    renderer->drawRect(hx, hy, hw, hh, a(0xF000));
                    renderer->drawString(this->upload.c_str(), false, hx + 40, hy + 40, 25, tsl::style::color::ColorText);

                    renderer->drawRect(hx - 4, hy - 4, hw + 8, 4, a(highlightColor));
                    renderer->drawRect(hx - 4, hy + hh, hw + 8, 4, a(highlightColor));
                    renderer->drawRect(hx - 4, hy, 4, hh, a(highlightColor));
                    renderer->drawRect(hx + hw, hy, 4, hh, a(highlightColor));
                    break;
                }
                case UploadStage::Uploading: {
                    /* Progress Bar */
                    renderer->drawCircle(x + 15, y + ThumbnailHeight + 107, 7, true, a(0xf888));
                    renderer->drawRect(x + 15, y + ThumbnailHeight + 100, w - 30, 15, a(0xf888));
                    renderer->drawCircle(x + w - 15, y + ThumbnailHeight + 107, 7, true, a(0xf888));

                    if (this->progress > 0.0f) {
                        renderer->drawCircle(x + 15, y + ThumbnailHeight + 107, 7, true, tsl::style::color::ColorHighlight);
                        renderer->drawRect(x + 15, y + ThumbnailHeight + 100, (w - 30) * this->progress, 15, tsl::style::color::ColorHighlight);
                        if (this->progress >= 1.0f)
                            renderer->drawCircle(x + w - 15, y + ThumbnailHeight + 107, 7, true, tsl::style::color::ColorHighlight);
                    }
                    break;
                }
                case UploadStage::Done: {
                    /* Url */
                    renderer->drawString(this->text.c_str(), false, x + 15, y + ThumbnailHeight + 85, 20, tsl::style::color::ColorText, w);
                    break;
                }
                case UploadStage::Canceled: {
                    /* Url */
                    renderer->drawString("Canceled", false, x + 15, y + ThumbnailHeight + 85, 20, tsl::style::color::ColorText, w);
                    break;
                }
            }
        });

        rootFrame->setContent(drawer);

        return rootFrame;
    }

    void MainGui::update() {
        if (this->stage == UploadStage::Uploading && this->response.wait_for(10us) == std::future_status::ready) {
            this->text  = this->response.get();
            this->stage = UploadStage::Done;
        }
    }

    bool MainGui::handleInput(u64 keysDown, u64, touchPosition touchInput, JoystickPosition, JoystickPosition) {
        if ((keysDown & KEY_A) || false) {
            this->response = std::async(std::launch::async, &Hoster::Upload, GetDefaultHoster(), fileId, [this](size_t total, size_t current) -> bool {
                this->progress = static_cast<float>(current) / static_cast<float>(total);
                return this->stage == UploadStage::Canceled;
            });

            this->stage = UploadStage::Uploading;
            return true;
        }

        if (keysDown & KEY_B) {
            this->stage = UploadStage::Canceled;
            return false;
        }

        if ((keysDown & KEY_Y) || false) {
        }

        return false;
    }

}
