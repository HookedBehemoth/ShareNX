#include "album_adapter.hpp"

#include "../translation/translation.hpp"
#include "album_thumb.hpp"

#include <album.hpp>
#include <cmath>
#include <fmt/core.h>
#include <switch.h>

using namespace std::chrono_literals;

namespace album {

    AsyncThumbnailLoader::AsyncThumbnailLoader() {
        threadCreate(&this->self, &LoadThreadEntry, this, nullptr, 0x1000, 0x30, 1);
        this->exitFlag = false;
        threadStart(&this->self);
    }

    AsyncThumbnailLoader::~AsyncThumbnailLoader() {
        if (!this->exitFlag) {
            this->exitFlag = true;
            threadWaitForExit(&this->self);
            threadClose(&this->self);
        }
    }

    void AsyncThumbnailLoader::Queue(const ThumbnailLoadTask &task) {
        task.thumb->ready = false;
        this->loadQueue.push(task);
    }

    void AsyncThumbnailLoader::Load(const ThumbnailLoadTask &task) {
        auto thumb   = task.thumb;
        auto &fileId = task.fileId;

        thumb->fileId = &fileId;

        u64 w, h;
        CapsScreenShotDecodeOption opts;
        CapsScreenShotAttribute attrs;

        Result rc = capsLoadAlbumScreenShotThumbnailImageEx0(&w, &h, &attrs, &fileId, &opts, imgBuffer, ImageSize, workBuffer, WorkSize);

        if (R_FAILED(rc))
            return brls::Logger::error("Failed to load image with: 0x%x", rc);

        if (fileId.content == CapsAlbumFileContents_Movie) {
            /* Round video length to nearest full number. */
            u8 seconds         = std::round(static_cast<float>(attrs.length_x10) / 1000);
            thumb->frameCount  = attrs.frame_count / 1000;
            thumb->videoLength = fmt::format(~LENGTH_FMT, seconds);
        } else {
            thumb->frameCount  = 0;
            thumb->videoLength = "";
        }

        task.thumb->imageBuffer = std::make_unique<u8[]>(ImageSize);
        std::memcpy(task.thumb->imageBuffer.get(), imgBuffer, ImageSize);
        task.thumb->ready = true;
    }

    void AsyncThumbnailLoader::LoadThreadEntry(void *user) {
        auto ptr = static_cast<AsyncThumbnailLoader *>(user);
        while (!ptr->exitFlag) {
            if (ptr->loadQueue.size() > 0) {
                ptr->Load(ptr->loadQueue.front());
                ptr->loadQueue.pop();
            }
            svcSleepThread(5'000'000);
        }
        ptr->exitFlag = true;
    }

    ThumbnailAdapter::ThumbnailAdapter() {
        this->applyFilter([](const CapsAlbumEntry &entry) -> bool { return true; });
    }

    size_t ThumbnailAdapter::getItemCount() {
        return albumFilterList.size();
    }

    brls::View *ThumbnailAdapter::createView() {
        return new Thumbnail();
    }

    void ThumbnailAdapter::bindView(brls::View *view, int index) {
        auto imageHolder = static_cast<album::Thumbnail *>(view);

        auto task = ThumbnailLoadTask{
            .fileId = albumFilterList[index].get(),
            .thumb  = imageHolder,
        };

        brls::Logger::info("bindView: %d", index);

        this->loader.Queue(task);
    }

    bool ThumbnailAdapter::applyFilter(FilterFunc filter) {
        auto &entries = getAllEntries();
        albumFilterList.clear();
        albumFilterList.reserve(std::size(entries));

        /* TODO */
        for (auto &entry : entries)
            if (filter(entry))
                albumFilterList.emplace_back(entry.file_id);

        return true;
    }

}
