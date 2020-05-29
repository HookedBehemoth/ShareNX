#pragma once

#include <borealis.hpp>
#include <switch.h>

class AlbumView : public brls::View {
  private:
    bool hideBar = false;
    brls::Hint hint;

  protected:
    brls::Image image;
    bool shouldClose;

    AlbumView(const CapsAlbumFileId &fileId);

  public:
    void layout(NVGcontext *vg, brls::Style *style, brls::FontStash *stash);
    void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx);
    void willAppear(bool resetState = false) override;
    void willDisappear(bool resetState = false) override;
    brls::View *getDefaultFocus() final;
};
