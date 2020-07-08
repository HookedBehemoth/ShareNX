#include "filter_item.hpp"

#include "../translation/translation.hpp"
#include "../util/ns.hpp"

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

        auto title = ns::getApplicationName(titleId);

        brls::Logger::info("tid: %016lX: %s (%d)", titleId, title.c_str(), count);

        this->updateLabel(title.c_str(), count);
    }

    FilterListItem::FilterListItem(int count) {
        this->filter = [](const CapsAlbumEntry &entry) -> bool {
            return entry.file_id.application_id < 0x010000000000ffff;
        };

        brls::Logger::info("other: %d", count);

        this->updateLabel(~FILTER_OTHER, count);
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

        auto style = brls::Application::getStyle();
        this->setHeight(style->List.Item.height);

        View::registerAction("Apply", brls::Key::A, [this]() -> bool {
            brls::Logger::debug("applying filter: %s", this->label.c_str());
            return FilterListItem::adapter->applyFilter(this->filter);
        });
    }
}
