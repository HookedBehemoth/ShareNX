#include "MainApplication.hpp"

MainApplication *mainApp;
std::string path;
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
    if(Down & KEY_A) {
        this->uploadLayout->showUrl(uploadFile(path));
        this->uploadLayout->SetOnInput(std::bind(&MainApplication::onInput_back, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }
}
void MainApplication::onInput_back(u64 Down, u64 Up, u64 Held) {
    if(Down & KEY_B) {
        list();
    }
}