#include "gui_selector.hpp"

#include <constants.hpp>
#include <uploader.hpp>

namespace album {

    tsl::elm::Element *SelectorGui::createUI() {
        auto *rootFrame = new tsl::elm::OverlayFrame("ShareNX \uE134", VERSION);

        auto *list = new tsl::elm::List();

        for (const auto &hoster : GetHosterList()) {
            auto *elm = new tsl::elm::ListItem(hoster.name);
            elm->setClickListener([this, &hoster](u64 keys) {
                if (keys & KEY_A) {
                    SetDefaultHoster(hoster);
                    this->should_close = true;
                    return true;
                }
                return false;
            });
            list->addItem(elm);
        }

        rootFrame->setContent(list);

        return rootFrame;
    }

    void SelectorGui::update() {
        if (this->should_close)
            tsl::goBack();
    }

}
