#pragma once

#include <borealis.hpp>

namespace album {

    /* View separator. Not focusable. */
    class PopupSeparator final : public brls::View {
      public:
        PopupSeparator();

        void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) override final;
    };

    /* Basic focusable text view. */
    class PopupItem : public brls::View {
      private:
        std::string label;

      public:
        PopupItem(const std::string &label, std::function<bool()> onClick);

        void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) override final;
        brls::View *getDefaultFocus() override final;
    };

    class PopupView : public brls::BoxLayout {
      public:
        PopupView(brls::View *parent);

        void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) override final;

        PopupItem *addItem(const std::string &label, std::function<bool()> onClick);
        void addSeparator();
    };

}
