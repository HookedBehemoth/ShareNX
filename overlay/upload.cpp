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
#include "upload.hpp"

#include <sys/select.h>
/* Curl header needs sys/select.h */
#include <curl/curl.h>

#include <tesla.hpp>

#include "album_movie_reader.hpp"

#define R_STR(cmd, message) \
    ({                      \
        auto rc = (cmd);    \
        if (R_FAILED(rc))   \
            return message; \
    })

namespace web {

    size_t StringWrite(const char *contents, size_t size, size_t nmemb, std::string *userp) {
        userp->append(contents, size * nmemb);
        return size * nmemb;
    }

    std::string UploadImage(const CapsAlbumFileId &file_id) {
        std::unique_ptr<char[]> imgBuffer;

        /* Init curl. */
        CURL *curl = curl_easy_init();
        if (curl == nullptr)
            return "Failed to init curl";

        /* Make filename from timestamp. */
        char file_name[0x20];
        auto &date = file_id.datetime;
        std::snprintf(file_name, sizeof(file_name), "%04d.%02d.%02d_%02d:%02d:%02d.%s", date.year, date.month, date.day, date.hour, date.minute, date.second, file_id.content == CapsAlbumFileContents_ScreenShot ? "jpg" : "mp4");

        /* Init mime. */
        curl_mime *mime = curl_mime_init(curl);

        /* Make file part. */
        curl_mimepart *file_part = curl_mime_addpart(mime);
        curl_mime_filename(file_part, file_name);
        curl_mime_name(file_part, "fileToUpload");

        /* Associate data/callback. */
        if (file_id.content == CapsAlbumFileContents_ScreenShot) {
            /* Get file size. */
            u64 size = 0;
            R_STR(capsaGetAlbumFileSize(&file_id, &size), "Can't get Filesize");

            /* Allocate memory. 0x7d000 is the largest possible image size. */
            imgBuffer = std::make_unique<char[]>(size);

            /* Load image. */
            u64 actualSize = 0;
            R_STR(capsaLoadAlbumFile(&file_id, &actualSize, imgBuffer.get(), size), "Failed to load Image");

            /* Associate data. */
            curl_mime_data(file_part, imgBuffer.get(), size);
        } else if (file_id.content == CapsAlbumFileContents_Movie) {
            /* Start read. */
            R_STR(album::Start(file_id), "Couldn't load image");

            /* Associate callback. */
			curl_mime_data_cb(file_part, album::GetStreamSize(), album::Read, nullptr, nullptr, nullptr);
        } else {
            return "Unsupported file type";
        }

        curl_mimepart *part = curl_mime_addpart(mime);
        curl_mime_name(part, "curl");
        curl_mime_data(part, "1", CURL_ZERO_TERMINATED);

        std::string urlresponse = std::string();

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StringWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&urlresponse);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        curl_easy_setopt(curl, CURLOPT_URL, "https://lewd.pics/p/index.php");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        CURLcode res = CURLE_OK;
        tsl::hlp::doWithSmSession([&] {
            res = curl_easy_perform(curl);
        });

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        if (res != CURLE_OK) {
            urlresponse = "Curl failed " + std::to_string(res);
        } else if (http_code != 200) {
            FILE *f = fopen("sdmc:/wat.html", "w+a");
            if (f != nullptr) {
                fwrite(urlresponse.data(), 1, urlresponse.size(), f);
                fclose(f);
            }
            urlresponse = "Failed with " + std::to_string(http_code);
        } else if (urlresponse.size() > 0x30) {
            urlresponse = "Result too long";
        }

        album::Close();

        curl_mime_free(mime);
        curl_easy_cleanup(curl);

        return urlresponse;
    }

}
