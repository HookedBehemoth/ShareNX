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
#include "gui_error.hpp"

static char result_buffer[10];

ErrorGui::ErrorGui(const char *msg)
    : m_msg(msg), m_result() {}

ErrorGui::ErrorGui(Result rc) {
    this->m_result = nullptr;
    if (R_MODULE(rc) == 206) {
        if ( R_DESCRIPTION(rc) == 23) {
            this->m_msg = "File not found.";
            this->m_result = "Was it deleted?";
        } else {
            this->m_msg = "Unknown Album error.";
        }
    } else {
        this->m_msg = "Unknown error occured.";
    }
    if (this->m_result == nullptr) {
        std::snprintf(result_buffer, 10, "2%03d-%04d", R_MODULE(rc), R_DESCRIPTION(rc));
        this->m_result = result_buffer;
    }
}

tsl::elm::Element *ErrorGui::createUI() {
    auto rootFrame = new tsl::elm::OverlayFrame("ShareNX \uE134", VERSION);

    auto *custom = new tsl::elm::CustomDrawer([&](tsl::gfx::Renderer *drawer, u16 x, u16 y, u16 w, u16 h) {
        static s32 msg_width = 0;
        static s32 result_width = 0;
        if (msg_width == 0) {
            auto [width, height] = drawer->drawString(this->m_msg, false, 0, 0, 25, tsl::style::color::ColorTransparent);
            msg_width = width;

            if (this->m_result) {
                auto [width, height] = drawer->drawString(this->m_msg, false, 0, 0, 25, tsl::style::color::ColorTransparent);
                result_width = width;
            }
        }
        drawer->drawString("\uE150", false, x + ((w - 90) / 2), 300, 90, 0xffff);
        drawer->drawString(this->m_msg, false, x + ((w - msg_width) / 2), 380, 25, 0xffff);
        if (this->m_result) {
            drawer->drawString(this->m_result, false, x + ((w - result_width) / 2), 430, 25, 0xffff);
        }
    });

    rootFrame->setContent(custom);

    return rootFrame;
}
