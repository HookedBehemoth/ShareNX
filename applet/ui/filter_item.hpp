#pragma once

#include "album_adapter.hpp"

#include <borealis.hpp>
#include <switch.h>

namespace album {

    class FilterListItem : public brls::View {
      private:
        std::string label;
        bool checked = false;
        FilterFunc filter = [](const CapsAlbumEntry &entry) -> bool { return true; };
        inline static ThumbnailAdapter *adapter;

      public:
        FilterListItem();
        FilterListItem(CapsAlbumFileContents type);
        FilterListItem(CapsAlbumStorage storage);
        FilterListItem(u64 titleId, int count);
        FilterListItem(int count);

        void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) override final;
        brls::View *getDefaultFocus() override final;

        static void setAdapter(ThumbnailAdapter *adapter);

      private:
        void updateLabel(const char *title, int count);
    };

}
