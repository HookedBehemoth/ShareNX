#pragma once

#include <tesla.hpp>

namespace album {

    class SelectorGui : public tsl::Gui {
      private:
        std::vector<std::string> hoster_list;

      public:
        SelectorGui();

        virtual tsl::elm::Element *createUI() override;
        virtual void update() override;
    };

}
