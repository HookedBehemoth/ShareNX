#pragma once

#include "album_gui.hpp"

namespace album {

    class PhotoView : public AlbumView {
      public:
        PhotoView(const CapsAlbumFileId &fileId);
    };

}
