#pragma once

#include "brls_ext/sane_image.hpp"

#include <borealis.hpp>
#include <switch.h>

namespace album {

    class ThumbnailAdapter;

    class Thumbnail : public brls::SaneImage {
        friend class album::ThumbnailAdapter;

      private:
        const CapsAlbumFileId *file_id = nullptr;
        std::string videoLength;
        int frameCount = 0;
        NVGpaint shadow;

      public:
        Thumbnail();
        ~Thumbnail();

        void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) override final;
        View *getDefaultFocus() override final;
    };

}
