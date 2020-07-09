#pragma once

#include <tesla.hpp>

namespace album {

    class SelectorGui : public tsl::Gui {
      public:
        virtual tsl::elm::Element *createUI() override;
        virtual void update() override;
    };

}
