#pragma once

#include <borealis.hpp>

// A sidebar with multiple tabs
class PopupSeparator : public brls::View {
  public:
    PopupSeparator();

    void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) override;
};

class PopupView;

class PopupItem : public brls::View {
  private:
    std::string label;

    PopupView *popupView = nullptr;

  public:
    PopupItem(const std::string &label, PopupView *popupView, std::function<bool()> onClick);

    void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) override;

    brls::View *getDefaultFocus() override;

    void onFocusGained() override;
};

class PopupView : public brls::BoxLayout {
  public:
    PopupView(brls::View *parent);

    PopupItem *addItem(const std::string &label, std::function<bool()> onClick);
    void addSeparator();

    void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) override;
};
