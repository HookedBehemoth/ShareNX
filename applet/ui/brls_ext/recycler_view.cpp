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

#include "recycler_view.hpp"

namespace brls {

    RecyclerContentView::~RecyclerContentView() {
        if (this->adapter)
            delete this->adapter;
    }

    void RecyclerContentView::layout(NVGcontext *vg, Style *style, FontStash *stash) {
        if (this->showHeight == 0)
            return this->updateAll();

        auto remaining = itemCount - this->itemOffset;
        for (size_t i = 0; i < std::min({remaining, std::size(this->children)}); i++)
            this->updateChild(this->itemOffset + i, false);
    }

    void RecyclerContentView::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, Style *style, FrameContext *ctx) {
        for (auto &child : this->children)
            child->frame(ctx);
    }

    View *RecyclerContentView::getNextFocus(FocusDirection direction, void *parentUserdata) {
        auto currRow = this->index / this->gridWidth;
        auto itemRow = this->itemOffset / this->gridWidth;
        auto maxRow  = (this->itemCount + this->gridWidth - 1) / this->gridWidth;

        if (itemCount == 0)
            return nullptr;

        switch (direction) {
            case FocusDirection::RIGHT:
                if (this->index % this->gridWidth == this->gridWidth - 1 || this->index + 1 >= itemCount)
                    return nullptr;
                this->index++;
                break;
            case FocusDirection::LEFT:
                if (this->index % this->gridWidth == 0 || this->index <= 0)
                    return nullptr;
                this->index--;
                break;
            case FocusDirection::UP:
                /* Check if already at the top. */
                if (static_cast<int>(this->index - this->gridWidth) < 0)
                    return nullptr;

                this->index -= this->gridWidth;

                currRow = this->index / this->gridWidth;

                if (this->itemOffset > 0 && currRow == itemRow + 1) {
                    if (this->childOffset == 0)
                        childOffset += std::size(this->children);
                    this->childOffset -= this->gridWidth;
                    this->itemOffset -= this->gridWidth;

                    /* Update views at the top. */
                    for (size_t i = 0; i < std::min({this->gridWidth, itemCount}); i++)
                        this->updateChild(itemOffset + i, true);
                }
                break;
            case FocusDirection::DOWN:
                /* Check if already at the bottom. */
                if (this->index + this->gridWidth >= itemCount)
                    return nullptr;

                this->index += this->gridWidth;

                currRow = this->index / this->gridWidth;

                if (itemRow + gridHeight < maxRow && currRow == (itemRow + gridHeight - 2)) {
                    this->childOffset = (this->childOffset + this->gridWidth) % std::size(this->children);
                    this->itemOffset += this->gridWidth;
                    size_t count = this->gridWidth;

                    /* Reached the last row. Might need to draw less than gridWidth */
                    if (itemRow + gridHeight == maxRow - 1)
                        count = itemCount % this->gridWidth;

                    /* Update views at the bottom. */
                    for (size_t i = 0; i < count; i++)
                        this->updateChild(this->itemOffset + (this->gridHeight - 1) * this->gridWidth + i, true);
                }
                break;
        }

        return this->children[this->index % std::size(this->children)].get();
    }

    View *RecyclerContentView::getDefaultFocus() {
        // Find the first focusable view in or after last index
        for (size_t i = index; i < itemCount; i++) {
            View *newFocus = this->children[i % std::size(this->children)]->getDefaultFocus();

            if (newFocus)
                return newFocus;
        }

        // Find the first focusable view before last index
        for (size_t i = index; i > 0; i--) {
            View *newFocus = this->children[i % std::size(this->children)]->getDefaultFocus();

            if (newFocus)
                return newFocus;
        }

        return nullptr;
    }

    void RecyclerContentView::onChildFocusGained(View *child) {
        this->childFocused = true;

        View::onChildFocusGained(child);
    }

    void RecyclerContentView::onChildFocusLost(View *child) {
        this->childFocused = false;

        View::onChildFocusLost(child);
    }

    void RecyclerContentView::willAppear(bool resetState) {
        for (auto &child : this->children)
            child->willAppear(resetState);
    }

    void RecyclerContentView::willDisappear(bool resetState) {
        for (auto &child : this->children)
            child->willDisappear(resetState);
    }

    void RecyclerContentView::onWindowSizeChanged() {
        for (auto &child : this->children)
            child->onWindowSizeChanged();
    }

    void RecyclerContentView::setAdapter(RecyclerAdapter *adapter) {
        if (this->adapter)
            delete this->adapter;

        this->adapter = adapter;

        this->showHeight = 0;

        this->invalidate();
    }

    void RecyclerContentView::setSpacing(int spacing) {
        this->spacing = spacing;

        this->showHeight = 0;

        this->invalidate();
    }

    void RecyclerContentView::setChildSize(unsigned width, unsigned height) {
        this->childWidth  = width;
        this->childHeight = height;

        this->showHeight = 0;

        this->invalidate();
    }

    void RecyclerContentView::setMargins(int top, int right, int bottom, int left) {
        this->marginBottom = bottom;
        this->marginLeft   = left;
        this->marginRight  = right;
        this->marginTop    = top;

        this->showHeight = 0;

        this->invalidate();
    }

    void RecyclerContentView::updateAll() {
        this->showHeight = this->parent->getHeight();
        this->itemCount  = adapter->getItemCount();

        this->gridWidth  = this->width / (this->childWidth + this->spacing);
        this->gridHeight = showHeight / (this->childHeight + this->spacing) + 3;

        auto entriesHeight = ((itemCount + gridWidth - 1) / gridWidth) * (childHeight + spacing);
        auto height        = entriesHeight - spacing + this->marginTop + this->marginBottom;
        this->setHeight(height);

        auto childCount = std::min({this->itemCount, this->gridHeight * this->gridWidth});
        auto viewCount  = std::size(this->children);

        if (childCount < viewCount) {
            children.resize(childCount);
        } else if (childCount > viewCount) {
            auto diff = childCount - viewCount;
            while (diff-- > 0) {
                auto view = this->adapter->createView();
                view->setParent(this);
                children.emplace_back(view);
            }

            for (size_t i = 0; i < childCount; i++)
                this->updateChild(i, true);
        }
    }

    void RecyclerContentView::updateChild(size_t itemIndex, bool fetch) {
        auto childIndex = itemIndex % std::size(this->children);
        auto &child     = this->children[childIndex];

        /* Update boundaries. */
        auto childCol = itemIndex % gridWidth;
        auto childRow = itemIndex / gridWidth;
        auto childX   = this->getX() + this->marginLeft + childCol * (this->childWidth + this->spacing);
        auto childY   = this->getY() + this->marginTop + childRow * (this->childHeight + this->spacing);

        child->setBoundaries(childX, childY, childWidth, childHeight);

        /* Update content. */
        if (fetch)
            this->adapter->bindView(child.get(), itemIndex);

        /* Update layout. */
        child->invalidate();
    }

    /* Getting this part to work took as long as getting all the above to work. */

    RecyclerView::RecyclerView() {
        this->layout.setParent(this);

        this->setContentView(&this->layout);
    }

    RecyclerContentView &RecyclerView::get() {
        return this->layout;
    }

}
