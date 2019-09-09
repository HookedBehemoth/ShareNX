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
    this->url = elm::TextBlock::New(80,640,"",45);
    this->url->SetColor(theme.WHITE);
    this->screenShot = elm::Image::New(10,40,path);
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