#include "albumview.hpp"

namespace {

    void Upload(const CapsAlbumFileId &fileId) {
        Result rc = 0;
        u64 size;
        rc = capsaGetAlbumFileSize(&fileId, &size);
        brls::Logger::info("0x%x", rc);
        CapsAlbumEntry entry{size, fileId};

        WebCommonConfig cfg;
        rc = webShareCreate(&cfg, WebShareStartPage_Default);
        brls::Logger::info("0x%x", rc);
        rc = webConfigAddAlbumEntryAndMediaData(&cfg, &entry, nullptr, 0);
        brls::Logger::info("0x%x", rc);

        WebCommonReply reply;
        rc = webConfigShow(&cfg, &reply);
        brls::Logger::info("0x%x", rc);
    }

}

AlbumView::AlbumView(const CapsAlbumFileId &fileId) : View(), hint(nvgRGBf(1.f, 1.f, 1.f)) {
    this->registerAction("Hide", brls::Key::PLUS, [this] {this->hideBar = !this->hideBar; return true; });
    this->registerAction("Back", brls::Key::B, [] {brls::Application::popView(); return true; });
    this->registerAction("OK", brls::Key::A, [fileId] {
        Upload(fileId);
        return true;
    });
    this->registerAction("Delete", brls::Key::X, [this, fileId] {
        brls::Dialog *dialog = new brls::Dialog("The file will be deleted.");

        dialog->addButton("Cancel", [dialog](brls::View *view) { dialog->close(); });
        dialog->addButton("Delete", [this, dialog, fileId](brls::View *view) {
            dialog->close();
            capsaDeleteAlbumFile(&fileId);
        });

        dialog->open();
        return true;
    });

    this->hint.setParent(this);
    this->image.setParent(this);
}

void AlbumView::layout(NVGcontext *vg, brls::Style *style, brls::FontStash *stash) {
    this->image.setBoundaries(0, 0, 1280, 720);
    this->image.layout(vg, style, stash);

    unsigned hintWidth = this->width - style->AppletFrame.separatorSpacing * 2 - style->AppletFrame.footerTextSpacing * 2;

    this->hint.setBoundaries(
        this->x + this->width - hintWidth - style->AppletFrame.separatorSpacing - style->AppletFrame.footerTextSpacing,
        this->y + this->height - style->AppletFrame.footerHeight,
        hintWidth,
        style->AppletFrame.footerHeight);
    this->hint.invalidate();
}

void AlbumView::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {
    this->image.draw(vg, 0, 0, 1280, 720, style, ctx);

    if (!this->hideBar) {
        nvgFillColor(vg, a(nvgRGBAf(0, 0, 0, 0.83f)));
        nvgBeginPath(vg);
        nvgRect(vg, x, y + height - 72, width, 72);
        nvgFill(vg);

        this->hint.frame(ctx);
    }
}

void AlbumView::willAppear(bool resetState) {
    this->image.willAppear(resetState);
    this->hint.willAppear(resetState);
}

void AlbumView::willDisappear(bool resetState) {
    this->image.willDisappear(resetState);
    this->hint.willDisappear(resetState);
}

brls::View *AlbumView::getDefaultFocus() {
    return this;
}
