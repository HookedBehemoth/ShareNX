#include "upload_process.hpp"

#include <fmt/core.h>

using namespace std::chrono_literals;

namespace album {

    UploadProcess::UploadProcess(Hoster &_hoster, const CapsAlbumFileId &_fileId) : hoster(&_hoster), fileId(_fileId) {
        response = std::async(std::launch::async, &Hoster::Upload, hoster, fileId, [this](size_t total, size_t current) -> bool {
            this->progress = static_cast<float>(current) / static_cast<float>(total);
            return this->cancel;
        });

        this->text = fmt::format("Uploading to {}", hoster->name);
    }

    UploadProcess::~UploadProcess() { this->cancel = true; }

    void UploadProcess::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {
        if (!this->done && this->response.wait_for(10us) == std::future_status::ready) {
            this->done = true;
            this->text = this->response.get();
            brls::Logger::info("Upload finished: %s", this->text.c_str());
        }

        // Draw text
        nvgFillColor(vg, a(ctx->theme->textColor));
        nvgFontSize(vg, style->Label.regularFontSize);
        nvgFontFaceId(vg, ctx->fontStash->regular);

        nvgTextLineHeight(vg, 1.0f);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgBeginPath(vg);
        nvgText(vg, x + width / 2, y + height / 2, this->text.c_str(), nullptr);

        if (!this->done) {
            const unsigned barLength = width - 30;
            unsigned progress_y      = y + 0.8f * height;

            /* Progress bar background */
            /* In official software this is more transparent instead of brighter. */
            nvgFillColor(vg, a(nvgRGBAf(1.f, 1.f, 1.f, 0.5f)));
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x + 15, progress_y, barLength, 16, 8);
            nvgFill(vg);

            /* Progress bar */
            nvgFillColor(vg, a(nvgRGB(0x00, 0xff, 0xc8)));
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x + 15, progress_y, barLength * this->progress, 16, 8);
            nvgFill(vg);
        }
    }

}
