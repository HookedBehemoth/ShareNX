#pragma once

#include "../brls_ext/popup_hint.hpp"
#include "../popup_view.hpp"

#include <borealis.hpp>
#include <switch.h>

namespace album {

    class AlbumView : public brls::View {
      private:
        brls::PopupHint hint;
        PopupView popupView;
        std::string titleName;
        std::string dateString;

      protected:
        bool hideBar = false;
        int image = -1;
        NVGpaint imgPaint;

        AlbumView(const CapsAlbumFileId &fileId);
        ~AlbumView();

      public:
        void layout(NVGcontext *vg, brls::Style *style, brls::FontStash *stash);
        void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx);
        void willAppear(bool resetState = false) override;
        void willDisappear(bool resetState = false) override;
        brls::View *getDefaultFocus() final;

        const CapsAlbumFileId fileId;
    };
}
