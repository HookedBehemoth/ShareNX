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
#include "image_item.hpp"

#include <sys/select.h>
/* Curl header for global init on start. */
#include <curl/curl.h>

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

    .udp_tx_buf_size = 0,
    .udp_rx_buf_size = 0,

    .sb_efficiency = 1,

    .num_bsd_sessions = 0,
    .bsd_service_type = BsdServiceType_Auto,
};

static u8 img[THUMB_SIZE];

class ShareOverlay : public tsl::Overlay {
  private:
    Result rc = 0;
    CURLcode curl_res = CURLE_OK;
    const char *msg = nullptr;
    CapsAlbumFileId fileId;

  public:
    virtual void initServices() override {
        R_INIT(socketInitialize(&sockConf), "Failed to init socket!");
        R_INIT(capsaInitialize(), "Failed to init capture service!");
        R_INIT(fsInitialize(), "Failed to init fs!");
        R_INIT(fsdevMountSdmc(), "Failed to mount sdmc!");
        curl_res = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (curl_res != CURLE_OK) {
            return;
        }

        u64 size;
        rc = capsaGetLastOverlayMovieThumbnail(&this->fileId, &size, img, THUMB_SIZE);
        if (R_FAILED(rc) || size == 0 || this->fileId.application_id == 0) {
            msg = "No screenshot taken!";
            return;
        }

        u8 *work = new (std::nothrow) u8[WORK_SIZE];
        if (work == nullptr) {
            msg = "Out of memory!";
            return;
        }
        tsl::hlp::ScopeGuard work_guard([work] { delete[] work; });

        u64 w, h;
        rc = capsaLoadAlbumScreenShotThumbnailImage(&w, &h, &this->fileId, img, THUMB_SIZE, work, WORK_SIZE);
        if (R_FAILED(rc) || w != THUMB_WIDTH || h != THUMB_HEIGHT) {
            msg = "CapSrv error!";
            return;
        }
    }
    virtual void exitServices() override {
        curl_global_cleanup();
        fsdevUnmountAll();
        fsExit();
        capsaExit();
        socketExit();
    }

    virtual void onShow() override {}
    virtual void onHide() override {}

    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
        if (R_FAILED(rc)) {
            return std::make_unique<ErrorGui>(rc);
        } else if (curl_res != CURLE_OK) {
            return std::make_unique<ErrorGui>("curl error");
        } else if (msg != nullptr) {
            return std::make_unique<ErrorGui>(msg);
        } else {
            return std::make_unique<MainGui>(this->fileId, img);
        }
    }
};

int main(int argc, char **argv) {
    return tsl::loop<ShareOverlay>(argc, argv);
}
