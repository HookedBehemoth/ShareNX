#include "albumview.hpp"

#include "../translation/translation.hpp"
#include "sane_dropdown.hpp"
#include "upload_process.hpp"

#include <album.hpp>

namespace album {

    namespace {

        Result NintendoShareUpload(const CapsAlbumFileId &fileId) {
            u64 size;
            R_TRY(capsaGetAlbumFileSize(&fileId, &size));
            CapsAlbumEntry entry{size, fileId};

            WebCommonConfig cfg;
            R_TRY(webShareCreate(&cfg, WebShareStartPage_Default));
            R_TRY(webConfigAddAlbumEntryAndMediaData(&cfg, &entry, nullptr, 0));

            WebCommonReply reply;
            R_TRY(webConfigShow(&cfg, &reply));

            u32 shareResult = 0;
            R_TRY(webReplyGetSharePostResult(&reply, &shareResult));

            return shareResult;
        }

    }

    AlbumView::AlbumView(const CapsAlbumFileId &_fileId) : popupView(this), fileId(_fileId) {
        auto OpenDeleteDialog = [this] {
            brls::Dialog *dialog = new brls::Dialog(~DELETE_INFO);

            dialog->addButton(~CANCEL, [dialog](brls::View *view) { dialog->close(); });
            dialog->addButton(~DELETE, [this, dialog](brls::View *view) {
                dialog->close([fileId = this->fileId] {
                    brls::Application::popView(brls::ViewAnimation::FADE, [fileId] {
                        Result rc = capsaDeleteAlbumFile(&fileId);
                        if (R_FAILED(rc)) {
                            brls::Logger::error("Failed to delete file");
                        }
                    });
                });
            });

            dialog->open();
            return true;
        };

        auto OpenCopyDialog = [this] {
            brls::Dialog *dialog = new brls::Dialog(this->fileId.storage == CapsAlbumStorage_Sd ? ~COPY_SD_TO_NAND : ~COPY_NAND_TO_SD);

            dialog->addButton(~CANCEL, [dialog](brls::View *view) { dialog->close(); });
            dialog->addButton(~COPY, [this, dialog](brls::View *view) {
                Result rc          = capsaStorageCopyAlbumFile(&this->fileId, this->fileId.storage == CapsAlbumStorage_Nand ? CapsAlbumStorage_Sd : CapsAlbumStorage_Nand);
                std::string result = R_SUCCEEDED(rc) ? "Successfully copied!" : fmt::MakeString("Copy failed: 0x%x", rc);
                dialog->close([result] {
                    auto *dialog = new brls::Dialog(result);
                    dialog->addButton(~OK, [dialog](brls::View *) { dialog->close(); });
                    dialog->open();
                });
            });

            dialog->open();
            return true;
        };

        auto OpenPostDialog = [this] {
            /* Get all hosters */
            std::vector<album::Hoster> &hosterList = album::GetHosterList();

            std::vector<std::string> hosterNameList;
            hosterNameList.reserve(hosterList.size() + 1);
            for (auto &hoster : hosterList)
                hosterNameList.push_back(hoster.name);
            hosterNameList.push_back("Nintendo Share");

            auto *dropdown = new SaneDropdown(~UPLOADER, hosterNameList);
            dropdown->setCallback([this](int idx) {
                brls::Logger::info("Uploading file: idx: %d, date: %d.%02d.%02d", idx, fileId.datetime.year, fileId.datetime.month, fileId.datetime.day);
                std::vector<album::Hoster> &hosterList = album::GetHosterList();

                if (idx >= static_cast<ssize_t>(hosterList.size())) {
                    NintendoShareUpload(fileId);
                } else if (idx >= 0) {
                    brls::Dialog *dialog = new brls::Dialog(new album::UploadProcess(hosterList[idx], fileId));
                    dialog->addButton(~CANCEL, [](brls::View *) { brls::Application::popView(); });
                    dialog->setCancelable(true);
                    brls::Application::pushView(dialog);
                }
            });
            brls::Application::pushView(dropdown);
            return true;
        };

        this->registerAction(~HIDE, brls::Key::PLUS, [this] {this->hideBar = !this->hideBar; return true; });
        this->registerAction(~BACK, brls::Key::B, [] {brls::Application::popView(); return true; });
        this->registerAction(~POST, brls::Key::A, [this] {
            this->popupView.show([] {}, true, brls::ViewAnimation::SLIDE_LEFT);
            brls::Application::giveFocus(&this->popupView);
            return true;
        });
        this->registerAction(~DELETE, brls::Key::X, OpenDeleteDialog);

        this->popupView.addItem(~POST, OpenPostDialog);
        this->popupView.addSeparator();
        this->popupView.addItem(~COPY, OpenCopyDialog);
        this->popupView.addItem(~DELETE, OpenDeleteDialog);

        this->popupView.setParent(this);
        this->hint.setParent(this);
        this->image.setParent(this);

        auto &datetime = fileId.datetime;
        dateString     = fmt::MakeString("%s  %2d/%02d/%04d %02d:%02d",
                                     fileId.storage == CapsAlbumStorage_Nand ? ~NAND : ~SD,
                                     datetime.day, datetime.month, datetime.year,
                                     datetime.hour, datetime.minute);
    }

    void AlbumView::layout(NVGcontext *vg, brls::Style *style, brls::FontStash *stash) {
        this->image.setBoundaries(0, 0, 1280, 720);
        this->image.layout(vg, style, stash);

        unsigned hintWidth = this->width - style->AppletFrame.separatorSpacing * 2 - style->AppletFrame.footerTextSpacing * 2;

        this->hint.setBoundaries(
            this->x + this->width - hintWidth - style->AppletFrame.separatorSpacing - style->AppletFrame.footerTextSpacing,
            this->y + this->height - style->AppletFrame.footerHeight,
            hintWidth,
            style->AppletFrame.footerHeight);
        this->hint.invalidate();

        this->popupView.setBoundaries(this->x, this->y, width / 3, height);
        this->popupView.invalidate();
    }

    void AlbumView::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {
        this->image.draw(vg, 0, 0, 1280, 720, style, ctx);

        if (!this->hideBar) {
            nvgFillColor(vg, a(nvgRGBAf(0, 0, 0, 0.83f)));
            nvgBeginPath(vg);
            nvgRect(vg, x, y + height - 72, width, 72);
            nvgFill(vg);

            this->hint.frame(ctx);
        }

        this->popupView.frame(ctx);

        if (!this->popupView.isHidden()) {
            nvgFillColor(vg, a(brls::Application::getThemeValuesForVariant(brls::ThemeVariant_DARK)->textColor));
            nvgFontSize(vg, 15);
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgBeginPath(vg);
            nvgText(vg, x + 40, y + 75, this->dateString.c_str(), nullptr);
        }
    }

    void AlbumView::willAppear(bool resetState) {
        this->image.willAppear(resetState);
        this->hint.willAppear(resetState);
    }

    void AlbumView::willDisappear(bool resetState) {
        this->image.willDisappear(resetState);
        this->hint.willDisappear(resetState);
    }

    brls::View *AlbumView::getDefaultFocus() {
        return this;
    }

}
