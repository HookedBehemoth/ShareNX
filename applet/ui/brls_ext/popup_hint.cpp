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

#include "popup_hint.hpp"

#include <set>

namespace brls {

    void PopupHint::rebuildHints() {
        // Check if the focused element is still a child of the same parent as the hint view's
        {
            View *focusParent    = brls::Application::getCurrentFocus();
            View *hintBaseParent = this;

            while (focusParent != nullptr) {
                if (focusParent->getParent() == nullptr)
                    break;
                focusParent = focusParent->getParent();
            }

            while (hintBaseParent != nullptr) {
                if (hintBaseParent->getParent() == nullptr)
                    break;
                hintBaseParent = hintBaseParent->getParent();
            }

            if (focusParent != hintBaseParent)
                return;
        }

        // Empty the layout and re-populate it with new Labels
        this->clear(true);

        std::set<brls::Key> addedKeys; // we only ever want one action per key
        View *focusParent = brls::Application::getCurrentFocus();

        // Iterate over the view tree to find all the actions to display
        std::vector<brls::Action> actions;

        while (focusParent != nullptr) {
            for (auto &action : focusParent->getActions()) {
                if (action.hidden)
                    continue;

                if (addedKeys.find(action.key) != addedKeys.end())
                    continue;

                addedKeys.insert(action.key);
                actions.push_back(action);
            }

            focusParent = focusParent->getParent();
        }

        // Sort the actions
        std::stable_sort(actions.begin(), actions.end(),  );

        // Populate the layout with labels
        for (const brls::Action &action : actions) {
            std::string hintText = Hint::getKeyIcon(action.key) + "  " + action.hintText;

            brls::Label *label = new brls::Label(brls::LabelStyle::HINT, hintText);
            label->setColor(nvgRGBf(1.f, 1.f, 1.f));
            this->addView(label);
        }
    }

}
