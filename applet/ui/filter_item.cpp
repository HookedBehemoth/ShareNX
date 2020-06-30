#include "filter_item.hpp"

#include "../translation/translation.hpp"

#include <fmt/core.h>
#include <util/caps.hpp>

namespace album {

    FilterListItem::FilterListItem() {
        size_t count = album::getAllEntries().size();
        this->updateLabel(~FILTER_ALL, count);
    }

    FilterListItem::FilterListItem(CapsAlbumFileContents type) {
        this->filter = [type](const CapsAlbumEntry &entry) -> bool {
            return entry.file_id.content == type;
        };

        auto &entries = album::getAllEntries();

        auto title = type == CapsAlbumFileContents_ScreenShot ? ~FILTER_SCREENSHOTS : ~FILTER_VIDEOS;
        auto count = std::count_if(std::begin(entries), std::end(entries), this->filter);

        this->updateLabel(title, count);
    }

    FilterListItem::FilterListItem(CapsAlbumStorage storage) {
        this->filter = [storage](const CapsAlbumEntry &entry) -> bool {
            return entry.file_id.storage == storage;
        };

        auto &entries = album::getAllEntries();

        auto title = storage == CapsAlbumStorage_Nand ? ~FILTER_NAND : ~FILTER_SD;
        auto count = std::count_if(std::begin(entries), std::end(entries), this->filter);

        this->updateLabel(title, count);
    }

    FilterListItem::FilterListItem(u64 titleId, int count) {
        this->filter = [titleId](const CapsAlbumEntry &entry) -> bool {
            return entry.file_id.application_id == titleId;
        };

        u64 size;
        auto *data = new (std::nothrow) NsApplicationControlData;
        if (data == nullptr) {
            this->label = ~ERROR;
            return;
        }

        const char *title = nullptr;
        NacpLanguageEntry *languageEntry;
        if (R_SUCCEEDED(nsGetApplicationControlData(NsApplicationControlSource_Storage, titleId, data, sizeof(NsApplicationControlData), &size)) &&
            R_SUCCEEDED(nacpGetLanguageEntry(&data->nacp, &languageEntry))) {
            title = languageEntry->name;
        } else {
            title = ~FILTER_OTHER;
        }
        brls::Logger::info("tid: %016lX: %s (%d)", titleId, title, count);

        this->updateLabel(title, count);
    }

    void FilterListItem::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {
        unsigned leftPadding = style->List.Item.padding;

        // Label
        nvgFillColor(vg, a(ctx->theme->textColor));
        nvgFontSize(vg, style->Label.listItemFontSize);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, ctx->fontStash->regular);
        nvgBeginPath(vg);
        nvgText(vg, x + leftPadding, y + this->height / 2, this->label.c_str(), nullptr);

        // Separators
        // Offset by one to be hidden by highlight
        nvgFillColor(vg, a(ctx->theme->listItemSeparatorColor));
        nvgBeginPath(vg);
        nvgRect(vg, x, y + 1 + this->height, width, 1);
        nvgFill(vg);
    }

    brls::View *FilterListItem::getDefaultFocus() {
        return this;
    }

    void FilterListItem::setAdapter(ThumbnailAdapter *adapter) {
        FilterListItem::adapter = adapter;
    }

    void FilterListItem::updateLabel(const char *title, int count) {
        
        this->label = fmt::format("{} ({})", title, count);
    }
}
