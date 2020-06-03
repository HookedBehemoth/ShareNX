#pragma once

#include "../translation/translation.hpp"

#include <borealis.hpp>
#include <switch.h>
#include <util/caps.hpp>
#include <util/fmt.hpp>

class FilterListItem : public brls::ListItem {
  public:
    FilterListItem() : ListItem("") {
        size_t count = album::getAllEntries().size();
        this->label  = fmt::MakeString("%s (%d)", ~FILTER_ALL, count);
    }

    FilterListItem(CapsAlbumFileContents type) : ListItem("") {
        size_t count = 0;
        for (const auto &entry : album::getAllEntries()) {
            count += entry.file_id.content == type;
        }
        this->label = fmt::MakeString("%s (%d)", type == CapsAlbumFileContents_ScreenShot ? ~FILTER_SCREENSHOTS : ~FILTER_VIDEOS, count);
    }

    FilterListItem(CapsAlbumStorage storage) : ListItem("") {
        size_t count = 0;
        for (const auto &entry : album::getAllEntries()) {
            count += entry.file_id.storage == storage;
        }
        this->label = fmt::MakeString("%s (%d)", storage == CapsAlbumStorage_Nand ? ~FILTER_NAND : ~FILTER_SD, count);
    }

    FilterListItem(u64 titleId, int count) : ListItem("") {
        u64 size;
        auto data = std::make_unique<NsApplicationControlData>();

        const char *appName = nullptr;
        NacpLanguageEntry *languageEntry;
        if (R_SUCCEEDED(nsGetApplicationControlData(NsApplicationControlSource_Storage, titleId, data.get(), sizeof(NsApplicationControlData), &size)) &&
            R_SUCCEEDED(nacpGetLanguageEntry(&data->nacp, &languageEntry))) {
            appName = languageEntry->name;
        } else {
            appName = ~FILTER_OTHER;
        }
        brls::Logger::info("tid: %016lX: %s (%d)", titleId, appName, count);

        this->label = fmt::MakeString("%s (%d)", appName, count);
    }
};