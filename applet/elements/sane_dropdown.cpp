/*
    Borealis, a Nintendo Switch UI Library
    Copyright (C) 2019-2020  natinusala
    Copyright (C) 2019-2020  p-sam

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "sane_dropdown.hpp"

#include "../translation/translation.hpp"

#include <borealis/animations.hpp>
#include <borealis/application.hpp>
#include <borealis/logger.hpp>

namespace album {

#define SELECT_VIEW_MAX_ITEMS 6 // for max height

    SaneDropdown::SaneDropdown(std::string title, std::vector<std::string> values)
        : title(title) {
        brls::Style *style = brls::Application::getStyle();

        this->topOffset = (float)style->Dropdown.listPadding / 8.0f;

        this->valuesCount = values.size();

        this->list = new brls::List(0);
        this->list->setParent(this);
        this->list->setMargins(1, 0, 1, 0);

        for (size_t i = 0; i < values.size(); i++) {
            std::string value = values[i];

            brls::ListItem *item = new brls::ListItem(value);

            item->setHeight(style->Dropdown.listItemHeight);
            item->setTextSize(style->Dropdown.listItemTextSize);

            item->getClickEvent()->subscribe([this, i](View *view) {
                brls::Application::popView(brls::ViewAnimation::FADE, [this, i] {
                    this->cb(i);
                });
            });

            this->list->addView(item);
        }

        this->hint = new brls::Hint();
        this->hint->setParent(this);

        this->registerAction(~BACK, brls::Key::B, [] {
            brls::Application::popView();
            return false;
        });
    }

    void SaneDropdown::show(std::function<void(void)> cb, bool animate, brls::ViewAnimation animation) {
        brls::View::show(cb);

        brls::menu_animation_ctx_entry_t entry;

        entry.duration     = this->getShowAnimationDuration(animation);
        entry.easing_enum  = brls::EASING_OUT_QUAD;
        entry.subject      = &this->topOffset;
        entry.tag          = (uintptr_t) nullptr;
        entry.target_value = 0.0f;
        entry.tick         = [this](void *userdata) { this->invalidate(); };
        entry.userdata     = nullptr;

        brls::menu_animation_push(&entry);
    }

    void SaneDropdown::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {
        unsigned top = this->list->getY() - style->Dropdown.headerHeight - style->Dropdown.listPadding;

        // Backdrop
        nvgFillColor(vg, a(ctx->theme->dropdownBackgroundColor));
        nvgBeginPath(vg);
        nvgRect(vg, x, y, width, top);
        nvgFill(vg);

        // TODO: Shadow

        // Background
        nvgFillColor(vg, a(ctx->theme->sidebarColor));
        nvgBeginPath(vg);
        nvgRect(vg, x, top, width, height - top);
        nvgFill(vg);

        // List
        this->list->frame(ctx);

        // Footer
        this->hint->frame(ctx);

        nvgFillColor(vg, ctx->theme->separatorColor); // we purposely don't apply opacity

        nvgBeginPath(vg);
        nvgRect(vg, x + style->AppletFrame.separatorSpacing, y + height - style->AppletFrame.footerHeight, width - style->AppletFrame.separatorSpacing * 2, 1);
        nvgFill(vg);

        nvgFillColor(vg, ctx->theme->textColor); // we purposely don't apply opacity
        nvgFontSize(vg, style->AppletFrame.footerTextSize);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgBeginPath(vg);
        nvgText(vg, x + style->AppletFrame.separatorSpacing + style->AppletFrame.footerTextSpacing, y + height - style->AppletFrame.footerHeight / 2, brls::Application::getCommonFooter()->c_str(), nullptr);

        // Header
        nvgFillColor(vg, a(ctx->theme->separatorColor));
        nvgBeginPath(vg);
        nvgRect(vg, x + style->AppletFrame.separatorSpacing, top + style->Dropdown.headerHeight - 1, width - style->AppletFrame.separatorSpacing * 2, 1);
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgFillColor(vg, a(ctx->theme->textColor));
        nvgFontFaceId(vg, ctx->fontStash->regular);
        nvgFontSize(vg, style->Dropdown.headerFontSize);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgText(vg, x + style->Dropdown.headerPadding, top + style->Dropdown.headerHeight / 2, this->title.c_str(), nullptr);
    }

    unsigned SaneDropdown::getShowAnimationDuration(brls::ViewAnimation animation) {
        return View::getShowAnimationDuration(animation) / 2;
    }

    void SaneDropdown::layout(NVGcontext *vg, brls::Style *style, brls::FontStash *stash) {
        // Layout and move the list
        unsigned listHeight = std::min(SELECT_VIEW_MAX_ITEMS, this->valuesCount) * style->Dropdown.listItemHeight - (unsigned)this->topOffset;
        unsigned listWidth  = style->Dropdown.listWidth + style->List.marginLeftRight * 2;

        this->list->setBoundaries(
            this->width / 2 - listWidth / 2,
            this->height - style->AppletFrame.footerHeight - listHeight - style->Dropdown.listPadding + (unsigned)this->topOffset,
            listWidth,
            listHeight);
        this->list->invalidate();

        // Hint
        // TODO: convert the bottom-left footer into a Label to get its width and avoid clipping with the hint
        unsigned hintWidth = this->width - style->AppletFrame.separatorSpacing * 2 - style->AppletFrame.footerTextSpacing * 2;

        this->hint->setBoundaries(
            this->x + this->width - hintWidth - style->AppletFrame.separatorSpacing - style->AppletFrame.footerTextSpacing,
            this->y + this->height - style->AppletFrame.footerHeight,
            hintWidth,
            style->AppletFrame.footerHeight);
        this->hint->invalidate();
    }

    brls::View *SaneDropdown::getDefaultFocus() {
        return this->list->getDefaultFocus();
    }

    void SaneDropdown::willAppear(bool resetState) {
        if (this->list)
            this->list->willAppear(resetState);

        if (this->hint)
            this->hint->willAppear(resetState);
    }

    void SaneDropdown::willDisappear(bool resetState) {
        if (this->list)
            this->list->willDisappear(resetState);

        if (this->hint)
            this->hint->willDisappear(resetState);
    }

    SaneDropdown::~SaneDropdown() {
        delete this->list;
        delete this->hint;
    }

}
