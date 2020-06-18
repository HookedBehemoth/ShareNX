#pragma once

#include <borealis.hpp>
#include <future>
#include <string>
#include <switch/services/caps.h>
#include <uploader.hpp>

namespace album {

    class UploadProcess : public brls::View {
      private:
        Hoster *hoster;
        const CapsAlbumFileId fileId;
        std::string text;
        std::future<std::string> response;
        bool has_data = false;
        std::string data;
        float progress = 0.0f;
        bool cancel = false;

      public:
        UploadProcess(Hoster &_hoster, const CapsAlbumFileId &_fileId);
        ~UploadProcess() { this->cancel = true; }

        void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) override;
        //void layout(NVGcontext *vg, brls::Style *style, brls::FontStash *stash) override;
    };

}
