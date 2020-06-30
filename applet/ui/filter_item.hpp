#pragma once

#include "album_adapter.hpp"

#include <borealis.hpp>
#include <switch.h>

namespace album {

    class FilterListItem : public brls::ListItem {
      private:
        FilterFunc filter = [](const CapsAlbumEntry &entry) -> bool { return true; };
        static ThumbnailAdapter *adapter;

      public:
        FilterListItem();
        FilterListItem(CapsAlbumFileContents type);
        FilterListItem(CapsAlbumStorage storage);
        FilterListItem(u64 titleId, int count);

        static void setAdapter(ThumbnailAdapter *adapter);

      private:
        void updateLabel(const char *title, int count);
    };

}
