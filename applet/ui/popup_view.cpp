#include "popup_view.hpp"

#include "../translation/translation.hpp"

namespace album {

    PopupSeparator::PopupSeparator() {
        auto style = brls::Application::getStyle();
        this->setHeight(style->Sidebar.Separator.height);
    }

    void PopupSeparator::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {
        nvgFillColor(vg, a(ctx->theme->sidebarSeparatorColor));
        nvgBeginPath(vg);
        nvgRect(vg, x, y + height / 2, width, 1);
        nvgFill(vg);
    }

    PopupItem::PopupItem(const std::string &label, std::function<bool()> onClick)
        : label(label) {
        auto style = brls::Application::getStyle();
        this->setHeight(style->Sidebar.Item.height);

        this->registerAction(~OK, brls::Key::A, onClick);
    }

    void PopupItem::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {
        nvgFillColor(vg, a(ctx->theme->textColor));
        nvgFontSize(vg, 25);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, ctx->fontStash->regular);
        nvgBeginPath(vg);
        nvgText(vg, x + style->Sidebar.Item.textOffsetX + style->Sidebar.Item.padding, y + height / 2, this->label.c_str(), nullptr);
    }

    brls::View *PopupItem::getDefaultFocus() {
        return this;
    }

    PopupView::PopupView(brls::View *parent)
        : BoxLayout(brls::BoxLayoutOrientation::VERTICAL) {
        this->registerAction(~BACK, brls::Key::B, [this, parent] {
            this->hide([] {}, true, brls::ViewAnimation::SLIDE_RIGHT);
            brls::Application::giveFocus(parent);
            return true;
        });
        auto style = brls::Application::getStyle();

        this->setWidth(460);
        this->setSpacing(style->Sidebar.spacing);
        this->setMargins(155, 40, 155, 40);
        this->setBackground(brls::Background::NONE);

        this->hide([] {}, false);
    }

    PopupItem *PopupView::addItem(const std::string &label, std::function<bool()> onClick) {
        auto item = new PopupItem(label, onClick);

        item->overrideThemeVariant(brls::Application::getThemeValuesForVariant(brls::ThemeVariant_DARK));
        this->addView(item);

        return item;
    }

    void PopupView::addSeparator() {
        auto separator = new PopupSeparator();
        this->addView(separator);
    }

    void PopupView::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {
        nvgFillColor(vg, a(nvgRGBAf(0, 0, 0, 0.83f)));
        nvgBeginPath(vg);
        nvgRect(vg, x, y, width, height);
        nvgFill(vg);

        BoxLayout::draw(vg, x, y, width, height, style, ctx);
    }

}
