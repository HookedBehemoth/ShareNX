#pragma once

#include <borealis.hpp>
#include <memory>
#include <switch.h>

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

class AlbumView : public brls::View {
  private:
    brls::Image image;

  public:
    AlbumView(const CapsAlbumFileId &fileId) : View() {
        this->registerAction("Back", brls::Key::B, [] {brls::Application::popView(); return true; });
        this->registerAction("OK", brls::Key::A, [fileId] {
            Upload(fileId);
            return true;
        });

        size_t workSize = 0x7d000;
        auto work = std::make_unique<u8[]>(workSize);
        size_t imgSize = 1280 * 720 * 4;
        auto img = std::make_unique<u8[]>(imgSize);

        u64 w, h;
        capsaLoadAlbumScreenShotImage(&w, &h, &fileId, img.get(), imgSize, work.get(), workSize);
        this->image.setRGBAImage(w, h, img.get());
        this->image.setParent(this);
    }

    void layout(NVGcontext *vg, brls::Style *style, brls::FontStash *stash) {
        this->image.setBoundaries(0, 0, 1280, 720);
        this->image.layout(vg, style, stash);
    }

    void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {
        this->image.draw(vg, 0, 0, 1280, 720, style, ctx);
    }

    brls::View *getDefaultFocus() final {
        return this;
    }
};
