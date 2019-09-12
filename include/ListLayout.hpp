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

using namespace pu::ui;
class ListLayout : public pu::ui::Layout
{
    public:
        ListLayout();
        PU_SMART_CTOR(ListLayout)
    private:
        void onItemClick();
        elm::TextBlock::Ref helloText;
        elm::Rectangle::Ref topRect;
        elm::Menu::Ref menu;
        elm::Image::Ref owo;
};