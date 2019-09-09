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