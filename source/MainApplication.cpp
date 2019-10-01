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
#include "MainApplication.hpp"

MainApplication *mainApp;
fs::path path;
int uploading = 0;
MainApplication::MainApplication() {
    mainApp = this;
    this->listLayout = ListLayout::New();
    this->LoadLayout(this->listLayout);
    this->listLayout->SetOnInput(std::bind(&MainApplication::onInput_list, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
void MainApplication::list() {
    this->LoadLayout(this->listLayout);
    
}
void MainApplication::upload(std::string selectedFile) {
    path = selectedFile;
    this->uploadLayout = UploadLayout::New();
    this->uploadLayout->SetOnInput(std::bind(&MainApplication::onInput_upload, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    this->LoadLayout(this->uploadLayout);
}
void MainApplication::onInput_list(u64 Down, u64 Up, u64 Held) {
    if((Down & KEY_PLUS) || (Down & KEY_MINUS) || (Down & KEY_B)) {
        this->Close();
    }
}
void MainApplication::onInput_upload(u64 Down, u64 Up, u64 Held) {
    if(Down & KEY_B) {
        list();
    }
    if(Down & KEY_A && !uploading) {
        uploaded = 1;
        this->uploadLayout->showUrl("Uploading... Please wait!");
        return;
    }
    if(uploading) {
        std::string url = getUrl(path);
        if(url.compare("")) {
            this->uploadLayout->showUrl(url);
            this->uploadLayout->SetOnInput(std::bind(&MainApplication::onInput_back, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        } else {
            this->uploadLayout->showUrl("Upload failed!");
        }
        uploaded = 0;
    }
}
void MainApplication::onInput_back(u64 Down, u64 Up, u64 Held) {
    if(Down & KEY_B) {
        list();
    }
}
