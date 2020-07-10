/*
 * Copyright (c) 2020 Behemoth
 *
 * This file is part of ShareNX.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#define TESLA_INIT_IMPL
#include "gui_error.hpp"
#include "gui_main.hpp"

#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <uploader.hpp>
#include <util/caps.hpp>
/* Curl header for global init on start. */
#include <curl/curl.h>
#include <constants.hpp>

#define R_INIT(cmd, message) \
    rc = (cmd);              \
    if (R_FAILED(rc)) {      \
        msg = message;       \
        return;              \
    }

constexpr const SocketInitConfig sockConf = {
    .bsdsockets_version = 1,

    .tcp_tx_buf_size = 0x800,
    .tcp_rx_buf_size = 0x800,
    .tcp_tx_buf_max_size = 0x25000,
    .tcp_rx_buf_max_size = 0x25000,

    .udp_tx_buf_size = 0x800,
    .udp_rx_buf_size = 0x800,

    .sb_efficiency = 1,

    .num_bsd_sessions = 0,
    .bsd_service_type = BsdServiceType_Auto,
};

namespace album {

    namespace {

        static u8 img[ThumbnailBufferSize];

    }

    class ShareOverlay : public tsl::Overlay {
      private:
        Result rc = 0;
        CURLcode curl_res = CURLE_OK;
        const char *msg = nullptr;
        u32 length;
        CapsAlbumFileId fileId;
        int nxlink;

      public:
        virtual void initServices() override {
            R_INIT(socketInitialize(&sockConf), "Failed to init socket!");
            nxlink = nxlinkStdio();
            R_INIT(capsaInitialize(), "Failed to init capture service!");
            album::InitializeHoster();
            curl_res = curl_global_init(CURL_GLOBAL_DEFAULT);
            if (curl_res != CURLE_OK) {
                return;
            }

            R_INIT(album::GetLatest(&this->fileId, img, ThumbnailBufferSize), "Failed to fetch remote.");

            if (this->fileId.application_id == 0) {
                msg = "No screenshot taken!";
                return;
            }

            size_t work_size = 0x10000;
            u8 *work = new (std::nothrow) u8[work_size];
            if (work == nullptr) {
                msg = "Out of memory!";
                return;
            }

            u64 w, h;
            if (hosversionBefore(4, 0, 0)) {
                rc = capsaLoadAlbumScreenShotThumbnailImage(&w, &h, &this->fileId, img, ThumbnailBufferSize, work, work_size);
                length = 0;
            } else {
                CapsScreenShotDecodeOption opts = {};
                CapsScreenShotAttribute attrs = {};
                rc = capsLoadAlbumScreenShotThumbnailImageEx0(&w, &h, &attrs, &this->fileId, &opts, img, ThumbnailBufferSize, work, work_size);
                length = attrs.length_x10;
            }

            if (R_FAILED(rc) || w != ThumbnailWidth || h != ThumbnailHeight) {
                msg = "CapSrv error!";
            }

            delete[] work;
        }
        virtual void exitServices() override {
            curl_global_cleanup();
            album::ExitHoster();
            capsaExit();
            ::close(nxlink);
            socketExit();
        }

        virtual void onShow() override {}
        virtual void onHide() override {}

        virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
            if (R_FAILED(rc)) {
                return std::make_unique<ErrorGui>(rc);
            } else if (curl_res != CURLE_OK) {
                return std::make_unique<ErrorGui>(curl_easy_strerror(curl_res));
            } else if (msg != nullptr) {
                return std::make_unique<ErrorGui>(msg);
            } else {
                return std::make_unique<MainGui>(this->fileId, img, length);
            }
        }
    };

}

int main(int argc, char **argv) {
    return tsl::loop<album::ShareOverlay>(argc, argv);
}
