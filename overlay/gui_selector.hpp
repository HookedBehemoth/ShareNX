#pragma once

#include <tesla.hpp>

namespace album {

    class SelectorGui : public tsl::Gui {
      private:
        bool should_close = false;

      public:
        virtual tsl::elm::Element *createUI() override;
        virtual void update() override;
    };

}
