#include "photo_gui.hpp"

namespace album {

    PhotoView::PhotoView(const CapsAlbumFileId &fileId) : AlbumView(fileId) {
        size_t workSize = 0x7d000;
        auto work       = std::make_unique<u8[]>(workSize);
        size_t imgSize  = 1280 * 720 * 4;
        auto img        = std::make_unique<u8[]>(imgSize);

        u64 w, h;
        capsaLoadAlbumScreenShotImage(&w, &h, &fileId, img.get(), imgSize, work.get(), workSize);
        nvgCreateImageRGBA(brls::Application::getNVGContext(), w, h, 0, img.get());
    }

}
