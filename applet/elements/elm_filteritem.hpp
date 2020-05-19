#pragma once

#include <borealis.hpp>
#include <switch.h>
#include <util/caps.hpp>
#include <util/fmt.hpp>

class FilterListItem : public brls::ListItem {
  public:
    FilterListItem() : ListItem("") {
        size_t count = album::getAllEntries().size();
        this->label = fmt::MakeString("%s (%d)", "All Screenshots and Videos", count);
    }
    FilterListItem(CapsAlbumFileContents type) : ListItem("") {
        size_t count = 0;
        for (const auto &entry : album::getAllEntries()) {
            count += entry.file_id.content == type;
        }
        this->label = fmt::MakeString("%s (%d)", type == CapsAlbumFileContents_ScreenShot ? "Screenshots" : "Videos", count);
    }

    FilterListItem(CapsAlbumStorage storage) : ListItem("") {
        size_t count = 0;
        for (const auto &entry : album::getAllEntries()) {
            count += entry.file_id.storage == storage;
        }
        this->label = fmt::MakeString("%s (%d)", storage == CapsAlbumStorage_Nand ? "System Memory" : "microSD Card", count);
    }

    FilterListItem(u64 titleId, int count) : ListItem("") {
        u64 size;
        NsApplicationControlData *data = new NsApplicationControlData();
        
        const char *appName = nullptr;
        NacpLanguageEntry *languageEntry;
        if (R_SUCCEEDED(nsGetApplicationControlData(NsApplicationControlSource_Storage, titleId, data, sizeof(NsApplicationControlData), &size)) &&
            R_SUCCEEDED(nacpGetLanguageEntry(&data->nacp, &languageEntry))) {
                appName = languageEntry->name;
        } else {
            appName = "Error";
        }
        brls::Logger::info("tid: %016lX: %s (%d)", titleId, appName, count);

        this->label = fmt::MakeString("%s (%d)", appName, count);

        delete data;
    }
};