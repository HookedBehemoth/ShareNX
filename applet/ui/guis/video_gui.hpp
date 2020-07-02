#pragma once

#include "../../util/video_decoder.hpp"
#include "album_gui.hpp"

namespace album {

    class MovieView : public AlbumView {
        static constexpr u32 ImageWidth       = 1280;
        static constexpr u32 ImageHeight      = 720;
        static constexpr u32 ImageComponent   = 4;
        static constexpr u32 DecodeBufferSize = ImageWidth * ImageHeight * ImageComponent;

      private:
        VideoDecoder decoder;
        u8 workBuffer[DecodeBufferSize];
        bool running = false;
        const u32 frameCount;

      public:
        MovieView(const CapsAlbumFileId &fileId, int frameCount);
        ~MovieView();

        void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx);
    };

}
