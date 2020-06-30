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

#pragma once

#include <borealis/event.hpp>
#include <borealis/hint.hpp>
#include <borealis/list.hpp>
#include <borealis/view.hpp>
#include <string>

namespace album {

    using DropdownCallback = std::function<void(int)>;

    class SaneDropdown : public brls::View {
      private:
        std::string title;

        int valuesCount;

        DropdownCallback cb;

        brls::List list;
        brls::Hint hint;

        float topOffset; // for slide in animation

      protected:
        unsigned getShowAnimationDuration(brls::ViewAnimation animation) override;

      public:
        SaneDropdown(const std::string &title, const std::vector<std::string> &values);
        ~SaneDropdown();

        void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) override;
        void layout(NVGcontext *vg, brls::Style *style, brls::FontStash *stash) override;
        View *getDefaultFocus() override;
        void show(std::function<void(void)> cb, bool animate = true, brls::ViewAnimation animation = brls::ViewAnimation::FADE) override;
        void willAppear(bool resetState = false) override;
        void willDisappear(bool resetState = false) override;

        bool isTranslucent() override {
            return true || View::isTranslucent();
        }

        inline void setCallback(DropdownCallback _cb) { this->cb = _cb; }
    };

}
