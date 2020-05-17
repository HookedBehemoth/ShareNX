#pragma once

#include <borealis.hpp>
#include <switch.h>

class LazyImage : public brls::View {
  private:
    CapsAlbumFileId file_id;
    brls::Image image;
    bool loaded = false;

  public:
    LazyImage(const CapsAlbumFileId &id);

    void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) final;
    void layout(NVGcontext *vg, brls::Style *style, brls::FontStash *stash) final;
    View *getDefaultFocus() final;

  private:
    void LoadImage();
};
