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

#include "ui/MainApplication.hpp"
#include "utils.hpp"

namespace scr::ui {
    MainApplication *mainApp;
    scr::utl::hosterConfig * m_config;

    void MainApplication::OnLoad() {
        mainApp = this;
        m_config = scr::utl::getDefaultConfig();
        

        this->listLayout = ListLayout::New();
        this->listLayout->SetOnInput(std::bind(&ListLayout::onInput, listLayout, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        this->LoadLayout(this->listLayout);
    }

    void MainApplication::upload(char * m_entry) {

    }
}