#include "gui_selector.hpp"

#include <constants.hpp>
#include <uploader.hpp>

namespace album {

    namespace {

        bool should_close = false;

    }

    SelectorGui::SelectorGui() {
        hoster_list = GetHosterNameList();
    }

    tsl::elm::Element *SelectorGui::createUI() {
        auto *rootFrame = new tsl::elm::OverlayFrame("ShareNX \uE134", VERSION);

        auto *list = new tsl::elm::List();

        const auto &current_hoster = GetHosterName();
        for (const auto &hoster : hoster_list) {
            auto *elm = new tsl::elm::ListItem(hoster);
            if (current_hoster == hoster) {
                elm->setValue("Selected");
            } else {
                elm->setClickListener([elm, hoster](u64 keys) {
                    if (keys & KEY_A) {
                        char path_buffer[FS_MAX_PATH];
                        std::snprintf(path_buffer, sizeof(path_buffer), "%s%s", HosterConfigPath, hoster.c_str());
                        if (LoadConfig(path_buffer)) {
                            should_close = true;
                        } else {
                            elm->setValue("failed");
                        }
                        return true;
                    }
                    return false;
                });
            }
            list->addItem(elm);
        }

        rootFrame->setContent(list);

        return rootFrame;
    }

    void SelectorGui::update() {
        if (should_close)
            tsl::goBack();
    }

}
