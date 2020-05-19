#pragma once

#include <borealis.hpp>
#include <switch.h>

class LazyImage : public brls::Image {
  private:
  u32 tick = 0;
    CapsAlbumFileId file_id;
    bool ready = false;
    bool loading = false;
    char *videoLength = nullptr;
    unsigned char *tmpBuffer = nullptr;

  public:
    LazyImage(const CapsAlbumFileId &id);
    ~LazyImage();

    void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) final;
    View *getDefaultFocus() final;

    void SetAlbumThumbnailImage(unsigned char* buffer, char *length);

  private:
    void LoadImage();
};
