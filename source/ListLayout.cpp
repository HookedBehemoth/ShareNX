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
#include "utils.hpp"

extern appcolor theme;
extern fs::path path;
extern MainApplication *mainApp;
std::vector<fs::path> *filePaths = new std::vector<fs::path>;
ListLayout::ListLayout() {
    std::string path = std::string("/" + getAlbumPath());
    this->SetBackgroundColor(theme.RED);
    this->SetBackgroundImage("romfs:/bg.jpg");
    this->topRect = elm::Rectangle::New(0,0,1280,30,theme.TOPBAR);
    this->helloText = elm::TextBlock::New(10,0,"Lewd.pics",25);
    this->helloText->SetColor(theme.WHITE);
    this->owo = elm::Image::New(989,240,"romfs:/owo.png");
    this->owo->SetWidth(291);
    this->owo->SetHeight(480);
    this->menu = elm::Menu::New(0,40,980,theme.RED,136,5,45);
    this->menu->SetOnFocusColor(theme.DRED);  
    for (auto &f: getDirectoryFiles(path, {".jpg", ".png", ".mp4"})){
        filePaths->push_back(f);
        auto itm = elm::MenuItem::New(f.filename().string().substr(0,12).insert(10,":").insert(8," ").insert(6,".").insert(4,"."));
        itm->SetColor(theme.WHITE);
        if (f.filename().extension() == ".mp4") itm->SetIcon("romfs:/video.png");
        else itm->SetIcon(f.string());
        itm->AddOnClick(std::bind(&ListLayout::onItemClick, this));
        menu->AddItem(itm);
    }
    this->Add(this->topRect);
    this->Add(this->helloText);
    this->Add(this->menu);
    this->Add(this->owo);
}
void ListLayout::onItemClick() {
    std::string path = (*filePaths)[this->menu->GetSelectedIndex()];
    mainApp->upload(path);
}
