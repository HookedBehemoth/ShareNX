#pragma once

#include <borealis.hpp>

namespace album {

    class PopupHint : public brls::Hint {
      protected:
        void rebuildHints() override;
    };

}
