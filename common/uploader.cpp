#include "uploader.hpp"

#include "album.hpp"

#include <cstring>
#include <json.hpp>
#include <memory>
#include <string>
#include <switch.h>
#include <sys/select.h>
/* Broken includes */
#include <curl/curl.h>

using json = nlohmann::json;
using namespace std::string_literals;

namespace album {

    namespace {

        size_t StringWrite(const char *contents, size_t size, size_t nmemb, std::string *userp) {
            userp->append(contents, size * nmemb);
            return size * nmemb;
        }

        int XferCallback(std::function<bool(size_t, size_t)> *userp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
            return (*userp)(ultotal, ulnow);
        }

        const Hoster lewd_pics = {
            .name      = "lewd.pics",
            .url       = "https://lewd.pics/p/index.php",
            .file_form = "fileToUpload",
            .scheme    = "",
            .body      = {{"curl", "1"}},
            .header    = {},
        };

        std::vector<Hoster> s_HosterList;

    }

    void InitializeHoster() {
        UpdateHoster();
    }

    void ExitHoster() {
    }

    const Hoster &GetDefaultHoster() {

        return lewd_pics;
    }

    void SetDefaultHoster(const Hoster &hoster) {
    }

    void UpdateHoster() {
        s_HosterList.clear();

        auto fs = fsdevGetDeviceFileSystem("sdmc");
        char path[FS_MAX_PATH];
        strcpy(path, HosterConfigPath);
        fsFsCreateDirectory(fs, path);

        Result rc;

        FsDir dir;
        if (R_FAILED(rc = fsFsOpenDirectory(fs, path, FsDirOpenMode_ReadFiles, &dir)))
            return;

        s64 count;
        FsDirectoryEntry entry;
        while (R_SUCCEEDED(fsDirRead(&dir, &count, 1, &entry)) && count) {
            size_t length = std::strlen(entry.name);
            if (length > 5 && std::strcmp(entry.name + length - 5, ".sxcu") == 0) {
                std::sprintf(path, "%s%s", HosterConfigPath, entry.name);
                FsFile file;
                if (R_SUCCEEDED(fsFsOpenFile(fs, path, FsOpenMode_Read, &file))) {
                    printf("parsing: %s\n", path);
                    auto &hoster = s_HosterList.emplace_back();
                    hoster.ParseFile(file, entry.file_size);
                    hoster.path = path;
                    fsFileClose(&file);
                }
            }
        }

        fsDirClose(&dir);
    }

    std::vector<Hoster> &GetHosterList() {
        return s_HosterList;
    }

    std::string Hoster::Upload(const CapsAlbumFileId &file_id, std::function<bool(size_t, size_t)> cb) {
        std::unique_ptr<char[]> imgBuffer;

        size_t progress = 0;

        /* Init curl. */
        CURL *curl = curl_easy_init();
        if (curl == nullptr)
            return "curl init failed";

        /* Init mime. */
        curl_mime *mime = curl_mime_init(curl);

        /* Make file part. */
        curl_mimepart *file_part = curl_mime_addpart(mime);
        curl_mime_name(file_part, this->file_form.c_str());

        /* Set arbitrary filename. */
        std::string file_name = MakeFileName(file_id);
        curl_mime_filename(file_part, file_name.c_str());

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
            curl_mime_type(file_part, "image/jpeg");
            curl_mime_data(file_part, imgBuffer.get(), size);
        } else if (file_id.content == CapsAlbumFileContents_Movie) {
            /* Start read. */
            R_STR(album::MovieReader::Start(file_id), "Failed to open movie stream");
            progress = album::MovieReader::GetProgress();
            album::MovieReader::seek(nullptr, 0, SEEK_SET);

            /* Associate callback. */
            curl_mime_type(file_part, "video/mp4");
            curl_mime_data_cb(file_part, album::MovieReader::GetStreamSize(), album::MovieReader::Read, nullptr, nullptr, nullptr);
        } else {
            curl_mime_free(mime);
            curl_easy_cleanup(curl);

            return "Unsupported file type";
        }

        /* Append body arguments. */
        for (auto &[k, v] : this->body) {
            /* Append header entry. String will be duped. */
            auto *arg = curl_mime_addpart(mime);
            curl_mime_name(arg, k.c_str());
            curl_mime_data(arg, v.c_str(), CURL_ZERO_TERMINATED);
        }

        /* Make custom http header. */
        curl_slist *chunk = NULL;
        for (const auto &line : this->header) {
            /* Append header entry. String will be duped. */
            chunk = curl_slist_append(chunk, line.c_str());
        }

        std::string urlresponse = std::string();

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StringWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &urlresponse);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &cb);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, XferCallback);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        smInitialize();
        CURLcode res = curl_easy_perform(curl);
        smExit();

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        curl_mime_free(mime);
        curl_easy_cleanup(curl);

        if (file_id.content == CapsAlbumFileContents_Movie) {
            /* Set read progess back. */
            album::MovieReader::seek(nullptr, progress, SEEK_SET);
            album::MovieReader::Close();
        }

        if (res == CURLE_OK) {
            if (http_code == 200) {
                try {
                    return this->ParseResponse(urlresponse);
                } catch (std::exception &e) {
                    return "Invalid response";
                }
            } else {
                return "Http Code: "s + std::to_string(http_code);
            }
        } else {
            return "curl error:\n"s + curl_easy_strerror(res);
        }
    }

    std::string Hoster::ParseResponse(const std::string &response) {
        std::string result;

        /* Return full reponse if no scheme was found. */
        if (this->scheme.empty())
            return response;

        /* Reserve at least size of scheme. */
        result.reserve(this->scheme.size());

        /* Get pointer to scheme data for easy traversal. */
        const char *ptr = this->scheme.c_str();

        /* Read while string goes on. */
        while (*ptr) {
            /* Append non-scheme characters. */
            while (*ptr != '$') {
                result += *ptr++;
            }

            if (std::memcmp(ptr, "$json:", 6) == 0) { /* Parse json. */
                ptr += 6;

                /* Parse response as json data. */
                json current = json::parse(response);

                /* Read while scheme is going. */
                while (*ptr != '$') {
                    std::string key;
                    if (isalpha(*ptr)) {
                        /* Append object key character. */
                        while (isalpha(*ptr)) {
                            key += *ptr++;
                        }
                        current = current[key];
                    } else if (*ptr == '[') {
                        ptr++;

                        /* Append array index character. */
                        while (isdigit(*ptr)) {
                            key += *ptr++;
                        }

                        /* Array index termination. */
                        if (*ptr++ == ']') {
                            current = current[std::strtoul(key.c_str(), nullptr, 10)];
                        } else {
                            return "Unexpected token in schema (Array termination)";
                        }
                    } else if (*ptr == '.') {
                        ptr++;
                    } else {
                        return "Unexpected token in schema (Array termination)";
                    }
                }
                result += current;
                ptr++;
            } else { /* Unsupported or unknown scheme type. */
                ptr++;
                while (*ptr++ != '$')
                    ;
                result += "UNSUPPORTED";
            }
        }
        return result;
    }

    void Hoster::ParseFile(FsFile &file, s64 file_size) {
        std::string text;
        text.resize(file_size);

        u64 size_read;
        if (R_SUCCEEDED(fsFileRead(&file, 0, text.data(), file_size, FsReadOption_None, &size_read))) {
            json j = json::parse(text.begin(), text.end());

            try {
                this->url       = j["RequestURL"];
                this->file_form = j["FileFormName"];

                if (j.contains("Name"))
                    this->name = j["Name"];
                if (j.contains("URL"))
                    this->scheme = j["URL"];

                if (j.contains("DestinationType")) {
                    std::string support = j["DestinationType"];
                    const char *str     = support.c_str();
                    while (*str) {
                        if (std::memcmp(str, "ImageUploader", 13) == 0) {
                            this->can_img = true;
                            str += 13;
                        } else if (std::memcmp(str, "VideoUploader", 13) == 0) {
                            this->can_mov = true;
                            str += 13;
                        } else {
                            str++;
                        }
                    }
                }

                /* Append parameters to URL. */
                bool first = true;
                for (auto &[k, v] : j["Parameters"].items()) {
                    this->url += first ? '?' : '&';
                    this->url += k + '=';
                    this->url += v;
                    first = false;
                }

                /* Append body arguments. */
                for (auto &[k, v] : j["Arguments"].items())
                    this->body.push_back({k, v.get<std::string>()});

                /* Make custom http header. */
                for (auto &[k, v] : j["Headers"].items())
                    this->header.push_back(k + v.get<std::string>());

            } catch (std::exception &e) {
                printf("failed to parse");
                this->name    = path + " (failed to parse)";
                this->can_img = false;
                this->can_mov = false;
            }
        }
    }
}
