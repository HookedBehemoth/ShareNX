#pragma once

#include "brls_ext/recycler_adapter.hpp"

#include <switch.h>

namespace album {

    using FilterFunc = std::function<bool(const CapsAlbumEntry &entry)>;

    class Thumbnail;

    class ThumbnailAdapter : public RecyclerAdapter {
        friend class album::Thumbnail;

      private:
        std::vector<std::reference_wrapper<const CapsAlbumFileId>> albumFilterList;

      public:
        ThumbnailAdapter();

        virtual size_t getItemCount() override final;
        virtual brls::View *createView() override final;
        virtual void bindView(brls::View *view, int index) override final;

      public:
        bool applyFilter(FilterFunc filter);
    };

}
