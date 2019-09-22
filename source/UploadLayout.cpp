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

extern std::string path;
extern appcolor theme;
UploadLayout::UploadLayout() {
    this->SetBackgroundColor(theme.RED);
    this->SetBackgroundImage("romfs:/bg.jpg");
    this->topRect = elm::Rectangle::New(0,0,1280,30,theme.TOPBAR);
    this->text = elm::TextBlock::New(10,0,"Lewd.pics",25);
    this->text->SetColor(theme.WHITE);
    this->url = elm::TextBlock::New(80,640,"Press A to upload, B to go back!",45);
    this->url->SetColor(theme.WHITE);
    if (path.find(".mp4") != std::string::npos) this->screenShot = elm::Image::New(10,40,"romfs:/video.png");
    else this->screenShot = elm::Image::New(10,40,path);
    this->screenShot->SetWidth(970);
    this->screenShot->SetHeight(545);
    this->owo = elm::Image::New(989,240,"romfs:/owo.png");
    this->owo->SetWidth(291);
    this->owo->SetHeight(480);
    this->Add(this->topRect);
    this->Add(this->text);
    this->Add(this->url);
    this->Add(this->screenShot);
    this->Add(this->owo);
}
void UploadLayout::showUrl(std::string url) {
    this->url->SetText(url);
}
