#pragma once
#include <pu/Plutonium>

using namespace pu::ui;
class ListLayout : public pu::ui::Layout
{
    public:
        ListLayout();
        PU_SMART_CTOR(ListLayout)
    private:
        elm::TextBlock::Ref helloText;
        elm::Rectangle::Ref topRect;
        elm::Menu::Ref menu;
        elm::Image::Ref owo;
};