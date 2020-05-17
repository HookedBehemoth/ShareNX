#include "elm_lazyimage.hpp"

LazyImage::LazyImage(const CapsAlbumFileId &id) : file_id(id) {
    this->setHeight(120);
    this->setWidth(210);
    this->registerAction("OK", brls::Key::A, [this] { brls::Application::notify("Click"); return true; });
}

void LazyImage::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {
    if (!loaded) {
        this->LoadImage();
    }

    float cornerRadius = (float)style->Button.cornerRadius;

    float shadowWidth   = style->Button.shadowWidth;
    float shadowFeather = style->Button.shadowFeather;
    float shadowOpacity = style->Button.shadowOpacity;
    float shadowOffset  = style->Button.shadowOffset;

    NVGpaint shadowPaint = nvgBoxGradient(vg,
        x, y + shadowWidth,
        width, height,
        cornerRadius * 2, shadowFeather,
        RGBA(0, 0, 0, shadowOpacity * alpha), brls::transparent);

    nvgBeginPath(vg);
    nvgRect(vg, x - shadowOffset, y - shadowOffset,
        width + shadowOffset * 2, height + shadowOffset * 3);
    nvgRoundedRect(vg, x, y, width, height, cornerRadius);
    nvgPathWinding(vg, NVG_HOLE);
    nvgFillPaint(vg, shadowPaint);
    nvgFill(vg);

    this->image.frame(ctx);
}

void LazyImage::layout(NVGcontext *vg, brls::Style *style, brls::FontStash *stash) {
    this->image.setBoundaries(x, y, 210, 120);
    this->image.invalidate();
}

brls::View* LazyImage::getDefaultFocus() {
    return this;
}

#include <memory>

void LazyImage::LoadImage() {
    size_t workSize = 0x10000;
    auto work = std::make_unique<u8[]>(workSize);
    size_t imgSize = 320 * 180 * 4;
    auto img = std::make_unique<u8[]>(imgSize);
    u64 w, h;
    Result rc = capsaLoadAlbumScreenShotThumbnailImage(&w, &h, &this->file_id, img.get(), imgSize, work.get(), workSize);
    if (R_SUCCEEDED(rc)) {
        this->image.setRGBAImage(w, h, img.get());
        loaded = true;
    } else
        brls::Logger::error("Failed to load image with: 0x%x", rc);
}
