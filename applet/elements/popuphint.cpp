#include "popuphint.hpp"

#include <set>

namespace album {

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
        std::stable_sort(actions.begin(), actions.end(), actionsSortFunc);

        // Populate the layout with labels
        for (const brls::Action &action : actions) {
            std::string hintText = Hint::getKeyIcon(action.key) + "  " + action.hintText;

            brls::Label *label = new brls::Label(brls::LabelStyle::HINT, hintText);
            label->setColor(nvgRGBf(1.f, 1.f, 1.f));
            this->addView(label);
        }
    }

}
