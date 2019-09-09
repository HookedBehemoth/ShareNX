#pragma once
#include <pu/Plutonium>

using namespace pu::ui;
class UploadLayout : public pu::ui::Layout
{
    public:
        UploadLayout();
        PU_SMART_CTOR(UploadLayout)
        void showUrl(std::string url);
    private:
        elm::TextBlock::Ref text;
        elm::TextBlock::Ref url;
        elm::Rectangle::Ref topRect;
        elm::Image::Ref screenShot;
        elm::Image::Ref owo;
};