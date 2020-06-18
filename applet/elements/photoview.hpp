#pragma once

#include "albumview.hpp"

#include <memory>

namespace album {

    class PhotoView : public AlbumView {
      public:
        PhotoView(const CapsAlbumFileId &fileId);
    };

}
