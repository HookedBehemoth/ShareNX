/*
    Borealis, a Nintendo Switch UI Library
    Copyright (C) 2019  natinusala
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

#include <borealis/scroll_view.hpp>
#include <vector>

class GridContentView : public brls::View {
  private:
    unsigned spacing   = 0;
    unsigned index     = 0;
    unsigned gridWidth = 1;

    unsigned childWidth;
    unsigned childHeight;

    bool resize = false; // should the view be resized according to children size after a layout?

  protected:
    std::vector<brls::View *> children;

    bool childFocused = false;

    unsigned marginTop    = 0;
    unsigned marginRight  = 0;
    unsigned marginBottom = 0;
    unsigned marginLeft   = 0;

  public:
    GridContentView();
    ~GridContentView();

    void layout(NVGcontext *vg, brls::Style *style, brls::FontStash *stash) override;
    void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) override;
    brls::View *getNextFocus(brls::FocusDirection direction, void *parentUserdata) override;
    brls::View *getDefaultFocus() override;
    void onChildFocusGained(brls::View *child) override;
    void onChildFocusLost(brls::View *child) override;
    void willAppear(bool resetState = false) override;
    void willDisappear(bool resetState = false) override;
    void onWindowSizeChanged() override;

    /**
      * Sets spacing between views
      */
    void setSpacing(unsigned spacing);
    unsigned getSpacing();

    /**
      * Sets margins around views
      */
    void setMargins(unsigned top, unsigned right, unsigned bottom, unsigned left);

    /**
     * Set size of child views
     */
    void setChildSize(unsigned width, unsigned height);

    /**
      * Adds a view to this box layout
      * If fill is set to true, the child will
      * fill the remaining space
      */
    void addView(brls::View *view);

    /**
      * Removes the view at specified
      * The view will be freed if free
      * is set to true (defaults to true)
      *
      * Warning: this method isn't correctly
      * implemented - currently removing a view will
      * most likely result in memory corruption
      */
    void removeView(int index, bool free = true);

    /**
     * Removes all views
     * from this layout
     */
    void clear(bool free = true);

    /**
      * Returns true if this layout
      * doesn't contain any views
      */
    bool isEmpty();

    bool isChildFocused();

    void setFocusedIndex(unsigned index);
    size_t getViewsCount();

    brls::View *getChild(size_t i);
};

class Grid : public brls::ScrollView {
  private:
    GridContentView *layout;

  public:
    Grid();
    ~Grid();

    // Wrapped GridContentView methods
    void addView(brls::View *view);
    void setMargins(unsigned top, unsigned right, unsigned bottom, unsigned left);
};
