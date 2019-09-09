#pragma once
#include <pu/Plutonium>
#include "ListLayout.hpp"

class MainApplication : public pu::ui::Application
{
    public:
        MainApplication();
        PU_SMART_CTOR(MainApplication)
    private:
        ListLayout::Ref listLayout;
};