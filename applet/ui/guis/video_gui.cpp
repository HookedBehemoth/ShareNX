#include "video_gui.hpp"

#include "../../translation/translation.hpp"

namespace album {

    MovieView::MovieView(const CapsAlbumFileId &fileId, int frameCount) : AlbumView(fileId), decoder(fileId), frameCount(frameCount) {
        this->registerAction(~PAUSE, brls::Key::Y, [this] {
            this->running = !this->running;
            this->updateActionHint(brls::Key::Y, this->running ? ~PAUSE : ~PLAY);
            return true;
        });

        brls::Application::setMaximumFPS(30);

        this->image = nvgCreateImageRGBA(brls::Application::getNVGContext(), ImageWidth, ImageHeight, 0, workBuffer);

        this->running = true;
    }

    MovieView::~MovieView() {
        this->running = false;

        nvgDeleteImage(brls::Application::getNVGContext(), this->image);

        brls::Application::setMaximumFPS(60);
    }

    void MovieView::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {
        if (this->focused && this->running) {
            auto frame = decoder.ReceiveFrame();
            if (frame != nullptr)
                nvgUpdateImageYUV(vg, this->image, frame->data, frame->linesize, workBuffer);
        }

        AlbumView::draw(vg, x, y, width, height, style, ctx);

        if (!this->hideBar) {
            /* Extra background */
            nvgFillColor(vg, a(nvgRGBAf(0, 0, 0, 0.83f)));
            nvgBeginPath(vg);
            nvgRect(vg, x, y + height - 72 - 14, width, 14);
            nvgFill(vg);

            const unsigned barLength = width - 30;

            /* Progress bar background */
            /* In official software this is more trasparent instead of brighter. */
            nvgFillColor(vg, a(nvgRGBAf(1.f, 1.f, 1.f, 0.5f)));
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x + 15, y + height - 72 - 6, barLength, 6, 4);
            nvgFill(vg);

            float progress = (float)(this->decoder.GetProgress() % this->frameCount) / (float)this->frameCount;

            /* Progress bar */
            nvgFillColor(vg, a(nvgRGB(0x00, 0xff, 0xc8)));
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x + 15, y + height - 72 - 6, barLength * progress, 6, 4);
            nvgFill(vg);
        }
    }

}
