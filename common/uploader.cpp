#include "uploader.hpp"

#include "album.hpp"

#include <cstring>
#include <memory>
#include <string>
#include <strings.h>
#include <switch.h>
#include <sys/select.h>
#include <tiny-json.h>
/* Broken includes */
#include <curl/curl.h>

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
        const Hoster *default_hoster = &lewd_pics;

        std::vector<Hoster> s_HosterList;

#ifdef __OVERLAY__
        FsFileSystem sdmc;
        inline FsFileSystem *getSdmc() {
            return &sdmc;
        }
#else
        inline FsFileSystem *getSdmc() {
            return fsdevGetDeviceFileSystem("sdmc");
        }
#endif

    }

    void InitializeHoster() {
#ifdef __OVERLAY__
        fsOpenSdCardFileSystem(&sdmc);
#endif
        UpdateHoster();
    }

    void ExitHoster() {
#ifdef __OVERLAY__
        fsFsClose(&sdmc);
#endif
    }

    const Hoster *GetDefaultHoster() {
        return default_hoster;
    }

    void SetDefaultHoster(const Hoster &hoster) {
        printf("setting default hoster: %s\n", hoster.name.c_str());
        default_hoster = &hoster;

        auto fs = getSdmc();
        char path[FS_MAX_PATH];
        std::sprintf(path, "%s%s", HosterConfigPath, HosterDefaultConfig);

        bool resize = false;
        Result rc;
        if (R_FAILED(rc = fsFsCreateFile(fs, path, hoster.path.size(), 0))) {
            if (rc != 0x402)
                return;

            resize = true;
        }

        FsFile file;
        if (R_FAILED(rc = fsFsOpenFile(fs, path, FsOpenMode_Write, &file))) {
            return;
        }

        if (resize && R_FAILED(rc = fsFileSetSize(&file, hoster.path.size()))) {
            return fsFileClose(&file);
        }

        fsFileWrite(&file, 0, hoster.path.c_str(), hoster.path.size(), FsWriteOption_Flush);
        return fsFileClose(&file);
    }

    void UpdateHoster() {
        s_HosterList.clear();

        auto fs = getSdmc();
        char path[FS_MAX_PATH];
        std::strcpy(path, HosterConfigPath);
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
                    auto &hoster = s_HosterList.emplace_back();
                    if (hoster.ParseFromFile(file, entry.file_size)) {
                        hoster.path = path;
                        if (hoster.name.size() == 0) {
                            entry.name[length - 5] = '\0';
                            hoster.name            = entry.name;
                        }
                        std::printf("%s parsed\n", hoster.name.c_str());
                    } else {
                        s_HosterList.erase(s_HosterList.end());
                        printf("failed to parse hoster from: %s\n", path);
                    }
                    fsFileClose(&file);
                }
            }
        }

        fsDirClose(&dir);

        std::sort(s_HosterList.begin(), s_HosterList.end(), [](const Hoster &lhs, const Hoster &rhs) {
            return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
        });

        /* Set old config as default. */
        std::sprintf(path, "%s%s", HosterConfigPath, HosterDefaultConfig);

        FsFile file;
        if (R_FAILED(fsFsOpenFile(fs, path, FsOpenMode_Read, &file))) {
            return;
        }

        u64 bytes_read;
        if (R_SUCCEEDED(fsFileRead(&file, 0, path, FS_MAX_PATH, FsReadOption_None, &bytes_read))) {
            auto it = std::find_if(s_HosterList.begin(), s_HosterList.end(), [path](const Hoster &hoster) {
                return hoster.path == path;
            });

            if (it != s_HosterList.end()) {
                default_hoster = &*it;
            }
        }

        return fsFileClose(&file);
    }

    std::vector<Hoster> &GetHosterList() {
        return s_HosterList;
    }

    std::string Hoster::Upload(const CapsAlbumFileId &file_id, std::function<bool(size_t, size_t)> cb) const {
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
                return this->ParseResponse(urlresponse);
            } else {
                return "Http Code: "s + std::to_string(http_code);
            }
        } else {
            return "curl error:\n"s + curl_easy_strerror(res);
        }
    }

    std::string Hoster::ParseResponse(const std::string &response) const {
        std::string result;
        printf("scheme: %s\n", this->scheme.c_str());

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

                /* Make modifyable copy of the response. */
                auto copy = response;
                json_t pool[0x40];

                /* Parse response as json data. */
                auto current = json_create(copy.data(), pool, 0x40);
                if (current == nullptr)
                    printf("response not json as expected: %s\n", response.c_str());

                /* Read while scheme is going and we still have parseable data. */
                while (*ptr != '$' && current) {
                    std::string key;
                    if (isalpha(*ptr)) {
                        /* Append object key character. */
                        while (isalpha(*ptr)) {
                            key += *ptr++;
                        }
                        printf("getting key: %s\n", key.c_str());
                        current = json_getProperty(current, key.c_str());
                    } else if (*ptr == '[') {
                        ptr++;

                        /* Append array index character. */
                        while (isdigit(*ptr)) {
                            key += *ptr++;
                        }

                        /* Array index termination. */
                        if (*ptr++ == ']') {
                            auto index = std::strtoul(key.c_str(), nullptr, 10);
                            current    = json_getChild(current);
                            while (index-- > 0 && current)
                                current = json_getSibling(current);
                        } else {
                            return "Unexpected token in schema (Array termination)";
                        }
                    } else if (*ptr == '.') {
                        ptr++;
                    } else {
                        return "Unexpected token in schema (Array termination)";
                    }
                }
                if (current == nullptr || json_getType(current) <= JSON_ARRAY)
                    return "Couldn't parse response json";
                result += json_getValue(current);
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

#define JSON_FOR_EACH(parent, name, FUN) ({            \
    auto _tmp_params = json_getProperty(parent, name); \
    if (_tmp_params) {                                 \
        auto entry = json_getChild(_tmp_params);       \
        while (entry) {                                \
            FUN;                                       \
            entry = json_getSibling(entry);            \
        }                                              \
    }                                                  \
})

    bool Hoster::ParseFromFile(FsFile &file, s64 file_size) {
        std::string text;
        text.resize(file_size);

        u64 size_read;
        if (R_FAILED(fsFileRead(&file, 0, text.data(), file_size, FsReadOption_None, &size_read))) {
            return false;
        }

        json_t pool[0x10];

        auto root = json_create(text.data(), pool, 0x10);
        if (root == nullptr)
            return false;

        const char *temp;

        temp = json_getPropertyValue(root, "RequestURL");
        if (!temp)
            return false;
        this->url = temp;

        temp = json_getPropertyValue(root, "FileFormName");
        if (!temp)
            return false;
        this->file_form = temp;

        /* Get display name. */
        temp = json_getPropertyValue(root, "Name");
        if (temp)
            this->name = temp;

        /* Get parse schema. */
        temp = json_getPropertyValue(root, "URL");
        if (temp)
            this->scheme = temp;

        /* Get supported types. */
        auto support = json_getPropertyValue(root, "DestinationType");
        if (support) {
            while (*support) {
                if (std::memcmp(support, "ImageUploader", 13) == 0) {
                    this->can_img = true;
                    support += 13;
                } else if (std::memcmp(support, "VideoUploader", 13) == 0) {
                    this->can_mov = true;
                    support += 13;
                } else {
                    support++;
                }
            }
        } else {
            /* Assume both. */
            this->can_img = true;
            this->can_mov = true;
        }

        /* Append parameters to URL. */
        bool first = true;
        JSON_FOR_EACH(root, "Parameters", {
            this->url += first ? '?' : '&';
            this->url += json_getName(entry) + '=';
            this->url += json_getValue(entry);
            first = false;
        });
        printf("url: %s\n", this->url.c_str());

        /* Append body arguments. */
        JSON_FOR_EACH(root, "Arguments", {
            this->body.emplace_back(json_getName(entry), json_getValue(entry));
            auto back = this->body.back();
            printf("body: %s: %s\n", back.first.c_str(), back.second.c_str());
        });

        /* Make custom http header. */
        JSON_FOR_EACH(root, "Headers", {
            this->header.push_back(std::move(std::string(json_getName(entry)).append(": ").append(json_getValue(entry))));
            printf("header: %s\n", this->header.back().c_str());
        });

        return true;
    }
}
