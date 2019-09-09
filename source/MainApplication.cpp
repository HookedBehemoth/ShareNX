#include <MainApplication.hpp>

MainApplication::MainApplication() {
    this->listLayout = ListLayout::New();
    this->LoadLayout(this->listLayout);
    this->SetOnInput([&](u64 Down, u64 Up, u64 Held, bool Touch) {
        if((Down & KEY_PLUS) || (Down & KEY_MINUS)) {
            this->Close();
        }
    });
}