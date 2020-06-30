/*
    Borealis, a Nintendo Switch UI Library
    Copyright (C) 2019  WerWolv
    Copyright (C) 2019  p-sam

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

#include "recycler_adapter.hpp"

#include <memory>
#include <borealis.hpp>

namespace brls {

    class RecyclerContentView : public View {
      private:
        size_t gridHeight = 1;
        size_t gridWidth  = 1;
        unsigned index    = 0;
        int spacing       = 0;

        unsigned childWidth  = 0;
        unsigned childHeight = 0;

        RecyclerAdapter *adapter = nullptr;

        std::vector<std::unique_ptr<View>> children;

        size_t showHeight = 0;

        size_t itemCount   = 0;
        size_t itemOffset  = 0;
        size_t childOffset = 0;

        bool childFocused = false;

        int marginTop    = 0;
        int marginRight  = 0;
        int marginBottom = 0;
        int marginLeft   = 0;

      public:
        RecyclerContentView()
            : childWidth(320), childHeight(180) {
            setSpacing(10);
            setMargins(0, 0, 0, 0);
        }
        ~RecyclerContentView();
        ;

        void layout(NVGcontext *vg, Style *style, FontStash *stash) override;
        void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, Style *style, FrameContext *ctx) override;
        View *getNextFocus(FocusDirection direction, void *parentUserdata) override;
        View *getDefaultFocus() override;
        void onChildFocusGained(View *child) override;
        void onChildFocusLost(View *child) override;
        void willAppear(bool resetState = false) override;
        void willDisappear(bool resetState = false) override;
        void onWindowSizeChanged() override;

        void setAdapter(RecyclerAdapter *adapter);
        void setSpacing(int spacing);
        void setChildSize(unsigned width, unsigned height);
        void setMargins(int top, int right, int bottom, int left);

      private:
        void updateAll();
        void updateChild(size_t index, bool fetch);
    };

    class RecyclerView : public ScrollView {
      private:
        RecyclerContentView layout;

      public:
        RecyclerView();

        RecyclerContentView &get();
    };

}
