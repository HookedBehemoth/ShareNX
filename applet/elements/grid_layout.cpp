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

#include "grid_layout.hpp"

#include <math.h>
#include <stdio.h>

#include <borealis/animations.hpp>
#include <borealis/application.hpp>
#include <borealis/logger.hpp>
#include <iterator>

namespace {

    constexpr int GridMarginLeftRight = 95;
    constexpr int GridMarginTop = 26;
    constexpr int GridMarginBottom = 23;
    constexpr int GridSpacing = 10;
    constexpr int GridSelectInset = 2;
    constexpr int GridItemWidth  = 210;
    constexpr int GridItemHeight = 120;
    constexpr int GridItemSelectRadius = 5;

}

GridContentView::GridContentView() {
    this->setMargins(GridMarginTop, GridMarginLeftRight, GridMarginBottom, GridMarginLeftRight);
    this->setChildSize(GridItemWidth, GridItemHeight);

    this->spacing = GridSpacing;
}

void GridContentView::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    // Draw children
    for (brls::View* child : this->children)
        if (this->parent->intersects(child))
            child->frame(ctx);
}

void GridContentView::setSpacing(unsigned spacing)
{
    this->spacing = spacing;
    this->invalidate();
}

unsigned GridContentView::getSpacing()
{
    return this->spacing;
}

void GridContentView::setMargins(unsigned top, unsigned right, unsigned bottom, unsigned left)
{
    this->marginBottom = bottom;
    this->marginLeft   = left;
    this->marginRight  = right;
    this->marginTop    = top;
    this->invalidate();
}

void GridContentView::setChildSize(unsigned width, unsigned height)
{
    this->childWidth = width;
    this->childHeight = height;
}

size_t GridContentView::getViewsCount()
{
    return this->children.size();
}

brls::View* GridContentView::getDefaultFocus()
{
    // Find the first focusable view in or after last index
    for (size_t i = index; i < this->children.size(); i++)
    {
        brls::View* newFocus = this->children[i]->getDefaultFocus();

        if (newFocus)
            return newFocus;
    }

    // Find the first focusable view before last index
    for (size_t i = index; i > 0; i--)
    {
        brls::View* newFocus = this->children[i]->getDefaultFocus();

        if (newFocus)
            return newFocus;
    }

    return nullptr;
}

brls::View* GridContentView::getNextFocus(brls::FocusDirection direction, void* parentUserData)
{
    switch (direction) {
        case brls::FocusDirection::RIGHT:
            if ((index % this->gridWidth) == (this->gridWidth - 1) || (index + 1) >= this->children.size())
                return nullptr;
            index++;
            break;
        case brls::FocusDirection::LEFT:
            if ((index % this->gridWidth) == 0 || index <= 0)
                return nullptr;
            index--;
            break;
        case brls::FocusDirection::UP:
            if (static_cast<int>(index - this->gridWidth) < 0)
                return nullptr;
            index -= this->gridWidth;
            break;
        case brls::FocusDirection::DOWN:
            if ((index + this->gridWidth) >= this->children.size())
                return nullptr;
            index += this->gridWidth;
            break;
    }
    return this->children[index];
}

void GridContentView::removeView(int index, bool free)
{
    brls::View* view = this->children[index];
    view->willDisappear();
    if (free)
        delete view;
    this->children.erase(this->children.begin() + index);
}

void GridContentView::clear(bool free)
{
    while (!this->children.empty())
        this->removeView(0, free);
}

void GridContentView::layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash)
{
    this->gridWidth = this->width / (this->childWidth + this->spacing);
    unsigned yAdvance = this->y + this->marginTop;

    unsigned entriesHeight = 0;

    for (size_t i = 0; i < this->children.size(); i += this->gridWidth)
    {
        unsigned xAdvance = this->marginLeft;
        unsigned remaining = this->children.size() - i;
        for (size_t j = 0; j < std::min(this->gridWidth, remaining); j++)
        {
            brls::View* child = this->children[i + j];
            child->setBoundaries(xAdvance,
                yAdvance,
                childWidth,
                childHeight);

            xAdvance += this->childWidth;
            xAdvance += this->spacing;

            child->layout(vg, style, stash); // call layout directly in case height is updated
        }
        yAdvance += this->spacing;
        yAdvance += this->childHeight;

        entriesHeight += this->spacing;
        entriesHeight += this->childHeight;
    }

    this->setHeight(entriesHeight - spacing + this->marginTop + this->marginBottom);
}

void GridContentView::addView(View* view)
{
    this->children.push_back(view);

    size_t position = this->children.size() - 1;

    size_t* userdata = (size_t*)malloc(sizeof(size_t));
    *userdata        = position;

    view->setParent(this, userdata);

    view->willAppear();
    this->invalidate();
}

brls::View* GridContentView::getChild(size_t index)
{
    return this->children[index];
}

bool GridContentView::isEmpty()
{
    return this->children.size() == 0;
}

bool GridContentView::isChildFocused()
{
    return this->childFocused;
}

void GridContentView::onChildFocusGained(View* child)
{
    this->childFocused = true;

    View::onChildFocusGained(child);
}

void GridContentView::onChildFocusLost(View* child)
{
    this->childFocused = false;

    View::onChildFocusLost(child);
}

GridContentView::~GridContentView()
{
    for (brls::View* child : this->children)
    {
        child->willDisappear();
        delete child;
    }

    this->children.clear();
}

void GridContentView::willAppear(bool resetState)
{
    for (brls::View* child : this->children)
        child->willAppear(resetState);
}

void GridContentView::willDisappear(bool resetState)
{
    for (brls::View* child : this->children)
        child->willDisappear(resetState);
}

void GridContentView::onWindowSizeChanged()
{
    for (brls::View* child : this->children)
        child->onWindowSizeChanged();
}

Grid::Grid() {
    this->layout = new GridContentView();

    this->layout->setParent(this);

    this->setContentView(this->layout);
}

Grid::~Grid()
{
    // ScrollView already deletes the content view
}

// Wrapped BoxLayout methods
void Grid::addView(brls::View* view)
{
    this->layout->addView(view);
}

void Grid::setMargins(unsigned top, unsigned right, unsigned bottom, unsigned left)
{
    this->layout->setMargins(
        top,
        right,
        bottom,
        left);
}
