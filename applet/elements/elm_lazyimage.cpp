#include "elm_lazyimage.hpp"

#include "../translation/translation.hpp"
#include "photoview.hpp"
#include "videoview.hpp"

#include <memory>
#include <mutex>
#include <queue>

namespace album {

    namespace {

        struct ImageLoaderTask {
            const CapsAlbumFileId &fileId;
            LazyImage *image;

            void make() const {
                size_t workSize = 0x10000;
                auto work       = std::make_unique<u8[]>(workSize);
                size_t imgSize  = 320 * 180 * 4;
                auto img        = new u8[imgSize];

                Result rc = 0;
                u64 w, h;
                char *videoLength = nullptr;
                int frameCount    = 0;
                if (hosversionBefore(4, 0, 0)) {
                    rc = capsaLoadAlbumScreenShotThumbnailImage(&w, &h, &this->fileId, img, imgSize, work.get(), workSize);
                } else {
                    CapsScreenShotDecodeOption opts = {};
                    CapsScreenShotAttribute attrs   = {};

                    rc = capsLoadAlbumScreenShotThumbnailImageEx0(&w, &h, &attrs, &this->fileId, &opts, img, imgSize, work.get(), workSize);

                    /* Round video length to nearest full number. */
                    u8 length = std::round(static_cast<float>(attrs.length_x10) / 1000);
                    if (length) {
                        videoLength = new char[8];
                        std::sprintf(videoLength, "%d%s", length, ~SECONDS_SHORT);
                        frameCount = attrs.frame_count / 1000;
                    }
                }
                if (R_SUCCEEDED(rc)) {
                    image->SetAlbumThumbnailImage(img, videoLength, frameCount);
                } else
                    brls::Logger::error("Failed to load image with: 0x%x", rc);
            }
        };

        class ImageLoader {
          private:
            Thread thread;
            std::queue<ImageLoaderTask> tasks;
            bool exitflag = false;
            std::mutex mtx;

            static void WorkThreadFunc(void *user) {
                ImageLoader *ptr = static_cast<ImageLoader *>(user);
                while (true) {
                    if (ptr->Loop())
                        continue;

                    {
                        std::scoped_lock lk(ptr->mtx);
                        if (ptr->exitflag)
                            break;
                    }

                    svcSleepThread(1'000'000);
                }
                ptr->exitflag = true;
            }

            bool Loop() {
                const ImageLoaderTask *task = nullptr;
                {
                    std::scoped_lock lk(mtx);
                    if (tasks.empty())
                        return false;

                    task = &tasks.front();
                }

                if (task) {
                    task->make();

                    std::scoped_lock lk(mtx);
                    tasks.pop();
                }
                return true;
            }

          public:
            ImageLoader() {
                threadCreate(&thread, WorkThreadFunc, this, nullptr, 0x1000, 0x2c, -2);
                threadStart(&thread);
            }

            ~ImageLoader() {
                if (!exitflag) {
                    exitflag = true;
                    threadWaitForExit(&thread);
                    threadClose(&thread);
                }
            }

            void Enqueue(const CapsAlbumFileId &fileId, LazyImage *image) {
                std::scoped_lock lk(mtx);
                tasks.emplace(fileId, image);
            }
        };

        ImageLoader g_loader;
    }

    LazyImage::LazyImage(const CapsAlbumFileId &id) : Image(), file_id(id) {
        this->registerAction(~OK, brls::Key::A, [this] {
            if (this->file_id.content == CapsAlbumFileContents_ScreenShot)
                brls::Application::pushView(new PhotoView(this->file_id));
            else
                brls::Application::pushView(new MovieView(this->file_id, this->frameCount));
            return true;
        });
        this->setScaleType(brls::ImageScaleType::SCALE);
    }

    LazyImage::~LazyImage() {
        if (this->videoLength)
            delete[] this->videoLength;
        if (tmpBuffer)
            delete[] tmpBuffer;
    }

    void LazyImage::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {
        if (!loading) {
            this->LoadImage();
        } else if (loading && ready) {
            this->setRGBAImage(320, 180, tmpBuffer);
            delete[] tmpBuffer;
            tmpBuffer = nullptr;
            this->layout(brls::Application::getNVGContext(), style, ctx->fontStash);
            this->invalidate();
            ready = false;
        }

        // Don't draw images out of view
        // TODO: Move to frame?
        auto *scrollview = this->parent->getParent();
        if (this->y + int(this->height) - scrollview->getY() < 0 || this->y > scrollview->getY() + int(scrollview->getHeight()))
            return;

        float cornerRadius = (float)style->Button.cornerRadius;

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

        Image::draw(vg, this->x, this->y, this->width, this->height, style, ctx);

        // Video length. 54x18
        if (this->videoLength) {
            nvgFillColor(vg, a(nvgRGBAf(0, 0, 0, 0.5f)));
            nvgBeginPath(vg);
            nvgRect(vg, x + width - 54, y + height - 18, 54, 18);
            nvgFill(vg);

            nvgFillColor(vg, a(nvgRGB(0xff, 0xff, 0xff)));
            nvgFontSize(vg, 14);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgBeginPath(vg);
            nvgText(vg, x + width - 27, y + height - 9, this->videoLength, nullptr);
            nvgFill(vg);
        }
    }

    brls::View *LazyImage::getDefaultFocus() {
        return this;
    }

    void LazyImage::SetAlbumThumbnailImage(unsigned char *buffer, char *length, int count) {
        this->tmpBuffer   = buffer;
        this->videoLength = length;
        this->frameCount  = count;
        this->ready       = true;
    }

    void LazyImage::LoadImage() {
        g_loader.Enqueue(this->file_id, this);
        this->loading = true;
    }

}
