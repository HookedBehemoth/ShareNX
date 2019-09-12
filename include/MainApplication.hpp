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
#include "ListLayout.hpp"
#include "UploadLayout.hpp"
#include "utils.hpp"

class MainApplication : public pu::ui::Application
{
    public:
        MainApplication();
        PU_SMART_CTOR(MainApplication)
        void list();
        void upload(std::string path);
        void onInput_list(u64 Down, u64 Up, u64 Held);
        void onInput_upload(u64 Down, u64 Up, u64 Held);
        void onInput_back(u64 Down, u64 Up, u64 Held);
    private:
        ListLayout::Ref listLayout;
        UploadLayout::Ref uploadLayout;
};