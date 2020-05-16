#include "image_item.hpp"

namespace album {

    ImageItem::ImageItem(const CapsAlbumFileId &file_id, const u8 *image, u32 video_length)
        : ListItem(""), fileId(file_id), buffer(image), img_x(), img_y(), url() {

        is_video = file_id.content == CapsAlbumFileContents_Movie;

        u8 seconds = (video_length + 999) / 1000;

        std::snprintf(this->length, 8, "%dsec", seconds);
        printf("video length: %dms, %ds\n", video_length, seconds);

        std::snprintf(this->appId, 0x11, "%016lX", this->fileId.application_id);

        std::snprintf(this->date, 0x20, "%4d:%02d:%02d %02d:%02d:%02d",
                      this->fileId.datetime.year,
                      this->fileId.datetime.month,
                      this->fileId.datetime.day,
                      this->fileId.datetime.hour,
                      this->fileId.datetime.minute,
                      this->fileId.datetime.second);
    }

    void ImageItem::draw(tsl::gfx::Renderer *renderer) {
        u32 img_x = this->getX() + ((this->getWidth() - ThumbnailWidth) / 2);
        renderer->drawRect(img_x, this->getY(), ThumbnailWidth, ThumbnailHeight, 0xf000);
        renderer->drawBitmap(img_x, this->getY(), ThumbnailWidth, ThumbnailHeight, this->buffer);

        if (is_video) {
            u32 w = (ThumbnailWidth / 4);
            u32 h = (ThumbnailHeight / 6);
            u32 x = img_x + ThumbnailWidth - w;
            u32 y = this->getY() + ThumbnailHeight - h;
            renderer->drawRect(x, y, w, h, tsl::Color(0, 0, 0, 0xa));
            renderer->drawString(this->length, false, x + 10, y + h - 4, 22, tsl::style::color::ColorText);
        }

        renderer->drawString(this->appId, false, this->getX() + 15, this->getY() + ThumbnailHeight + 25, 20, tsl::style::color::ColorText);
        renderer->drawString(this->date, false, this->getX() + 15, this->getY() + ThumbnailHeight + 55, 20, tsl::style::color::ColorText);

        if (!url.empty())
            renderer->drawString(this->url.c_str(), false, this->getX() + 15, this->getY() + ThumbnailHeight + 85, 20, tsl::style::color::ColorText);
    }

    void ImageItem::layout(u16, u16, u16, u16) {
        this->setBoundaries(this->getX(), this->getY(), this->getWidth(), ThumbnailHeight + 100);
    }

    tsl::elm::Element *ImageItem::requestFocus(tsl::elm::Element *, tsl::FocusDirection) {
        return nullptr;
    }

}
