#include "album_thumb.hpp"

#include "../translation/translation.hpp"
#include "guis/photo_gui.hpp"
#include "guis/video_gui.hpp"

namespace album {

    Thumbnail::Thumbnail() : SaneImage() {
        this->registerAction(~OK, brls::Key::A, [this] {
            try {
                if (this->fileId && this->fileId->content == CapsAlbumFileContents_ScreenShot)
                    brls::Application::pushView(new PhotoView(*this->fileId));
                else
                    brls::Application::pushView(new MovieView(*this->fileId, this->frameCount));
            } catch (String msg) {
                auto dialog = new brls::Dialog(~msg);
                dialog->addButton(~OK, [dialog](brls::View *) { dialog->close(); });
                dialog->open();
            }
            return true;
        });
    }

    void Thumbnail::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {
        if (!this->ready) {
            return;
        } else if (this->imageBuffer != nullptr) {
            this->setRGBAImage(imageBuffer.get(), 320, 180);
            this->invalidate(true);
            imageBuffer = nullptr;
        }

        float cornerRadius = style->Button.cornerRadius;

        float shadowWidth   = 2.0f;
        float shadowFeather = 10.0f;
        float shadowOpacity = 63.75f;
        float shadowOffset  = 10.0f;

        NVGpaint shadowPaint = nvgBoxGradient(vg,
                                              x, y + shadowWidth,
                                              width, height,
                                              cornerRadius * 2, shadowFeather,
                                              RGBA(0, 0, 0, shadowOpacity * alpha), brls::transparent);

        nvgBeginPath(vg);
        nvgRect(vg, x - shadowOffset, y - shadowOffset,
                width + shadowOffset * 2, height + shadowOffset * 3);
        nvgRoundedRect(vg, x, y, width, height, cornerRadius);
        nvgPathWinding(vg, NVG_HOLE);
        nvgFillPaint(vg, shadowPaint);
        nvgFill(vg);

        SaneImage::draw(vg, this->x, this->y, this->width, this->height, style, ctx);

        // Video length text. 54x18
        if (!this->videoLength.empty()) {
            nvgFillColor(vg, a(nvgRGBAf(0, 0, 0, 0.5f)));
            nvgBeginPath(vg);
            nvgRect(vg, x + width - 54, y + height - 18, 54, 18);
            nvgFill(vg);

            nvgFillColor(vg, a(nvgRGB(0xff, 0xff, 0xff)));
            nvgFontSize(vg, 14);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgBeginPath(vg);
            nvgText(vg, x + width - 27, y + height - 9, this->videoLength.c_str(), nullptr);
            nvgFill(vg);
        }
    }

    brls::View *Thumbnail::getDefaultFocus() {
        return this;
    }

}
