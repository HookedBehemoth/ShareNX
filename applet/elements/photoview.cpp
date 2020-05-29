#include "photoview.hpp"

PhotoView::PhotoView(const CapsAlbumFileId &fileId) : AlbumView(fileId) {
    size_t workSize = 0x7d000;
    auto work       = std::make_unique<u8[]>(workSize);
    size_t imgSize  = 1280 * 720 * 4;
    auto img        = std::make_unique<u8[]>(imgSize);

    u64 w, h;
    capsaLoadAlbumScreenShotImage(&w, &h, &fileId, img.get(), imgSize, work.get(), workSize);
    this->image.setRGBAImage(w, h, img.get());
}
