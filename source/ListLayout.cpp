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
    this->helloText = elm::TextBlock::New(10,0,path.c_str(),25);
    this->helloText->SetColor(theme.WHITE);
    this->owo = elm::Image::New(989,240,"romfs:/owo.png");
    this->owo->SetWidth(291);
    this->owo->SetHeight(480);
    this->menu = elm::Menu::New(0,40,980,theme.RED,136,5,45);
    this->menu->SetOnFocusColor(theme.DRED);  
    for (auto &f: getDirectoryFiles(path, {".jpg", ".png"})){
        filePaths->push_back(f);
        auto itm = elm::MenuItem::New(f.filename().string().substr(0,12).insert(10,":").insert(8," ").insert(6,".").insert(4,"."));
        itm->SetColor(theme.WHITE);
        itm->SetIcon(f.string());
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