#include "ListLayout.hpp"
#include "utils.hpp"
Color WHITE = Color::FromHex("#FFFFFFFF");
Color RED = Color::FromHex("#6c0000FF");
Color DRED = Color::FromHex("#480001FF");
Color TOPBAR = Color::FromHex("#170909FF");
ListLayout::ListLayout() {
    std::string path = std::string("/" + getAlbumPath());
    this->SetBackgroundColor(RED);
    this->SetBackgroundImage("romfs:/bg.jpg");
    this->topRect = elm::Rectangle::New(0,0,1280,30,TOPBAR);
    this->helloText = elm::TextBlock::New(0,0,path.c_str(),25);
    this->helloText->SetColor(WHITE);
    this->owo = elm::Image::New(989,240,"romfs:/owo.png");
    this->owo->SetWidth(291);
    this->owo->SetHeight(480);
    this->menu = elm::Menu::New(0,40,980,RED,136,5);
    this->menu->SetOnFocusColor(DRED);  
    for (auto &f: getDirectoryFiles(path, {".jpg", ".png"})){
        auto itm = elm::MenuItem::New(f.filename().string());
        itm->SetColor(WHITE);
        itm->SetIcon(f.string());
        menu->AddItem(itm);
    }
    this->Add(this->topRect);
    this->Add(this->helloText);
    this->Add(this->menu);
    this->Add(this->owo);
}