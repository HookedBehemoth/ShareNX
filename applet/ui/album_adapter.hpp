#pragma once

#include "brls_ext/recycler_adapter.hpp"

#include <queue>
#include <switch.h>
#include <thread>

namespace album {

    using FilterFunc = std::function<bool(const CapsAlbumEntry &entry)>;

    class Thumbnail;

    struct ThumbnailLoadTask {
        const CapsAlbumFileId &fileId;
        Thumbnail *thumb;
    };

    class AsyncThumbnailLoader {
        friend class Thumbnail;

      private:
        static constexpr const size_t WorkSize = 0x10000;
        static constexpr const size_t ImageSize  = 320 * 180 * 4;
        u8 workBuffer[WorkSize];
        u8 imgBuffer[ImageSize];

        bool exitFlag;
        Thread self; /* Sigh... */
        std::queue<ThumbnailLoadTask> loadQueue;

      public:
        AsyncThumbnailLoader();
        ~AsyncThumbnailLoader();

        void Queue(const ThumbnailLoadTask &task);
        void Load(const ThumbnailLoadTask &task);

      private:
        static void LoadThreadEntry(void *ptr);
    };

    class ThumbnailAdapter : public RecyclerAdapter {
      private:
        AsyncThumbnailLoader loader;
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
