#pragma once

#include "albumview.hpp"

#include <memory>

class PhotoView : public AlbumView {
  public:
    PhotoView(const CapsAlbumFileId &fileId);
};
