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

namespace album {

    namespace {

        struct Hoster {
            std::string name;
            bool can_img, can_mov;
            std::string url, file_form;
            std::string scheme;
            std::map<std::string, std::string> body;
            std::vector<std::string> header;
        };

        Hoster lewd_pics = {
            .name = "lewd.pics",
            .url = "https://lewd.pics/p/index.php",
            .file_form = "fileToUpload",
            .scheme = "",
            .body = {{"curl", "1"}},
            .header = {},
        };

        FsFileSystem sdmc;

        Hoster g_hoster = {};

        size_t StringWrite(const char *contents, size_t size, size_t nmemb, std::string *userp) {
            userp->append(contents, size * nmemb);
            return size * nmemb;
        }

        std::string ParseResponse(const std::string &response) {
            std::string result;

            /* Return full reponse if no scheme was found. */
            if (g_hoster.scheme.empty())
                return response;

            /* Reserve at least size of scheme. */
            result.reserve(g_hoster.scheme.size());

            /* Get pointer to scheme data for easy traversal. */
            const char *ptr = g_hoster.scheme.c_str();

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

    }

    void Initialize() {
        fsOpenSdCardFileSystem(&sdmc);

        {
            char path_buffer[FS_MAX_PATH];
            std::strcpy(path_buffer, HosterConfigPath);
            fsFsCreateDirectory(&sdmc, path_buffer);
        }

        LoadDefault();
    }

    void Exit() {
        fsFsClose(&sdmc);
    }

    const std::string &GetHosterName() {
        return g_hoster.url;
    }

    std::vector<std::string> GetHosterNameList() {
        std::vector<std::string> result;

        FsDirectoryEntry entry;
        std::strcpy(entry.name, HosterConfigPath);

        FsDir dir;
        if (R_SUCCEEDED(fsFsOpenDirectory(&sdmc, entry.name, FsDirOpenMode_ReadFiles, &dir))) {
            s64 read_count;
            while (R_SUCCEEDED(fsDirRead(&dir, &read_count, 1, &entry)) && read_count > 0) {
                result.push_back(entry.name);
            }
        }
        return result;
    }

    void LoadDefault() {
        char path_buffer[FS_MAX_PATH];
        std::snprintf(path_buffer, FS_MAX_PATH, "%s%s", HosterConfigPath, HosterDefaultConfig);
    
        FsFile file;
        if (R_SUCCEEDED(fsFsOpenFile(&sdmc, path_buffer, FsOpenMode_Read, &file))) {
            u64 bytes_read;
            size_t offset = std::strlen(HosterConfigPath);
            fsFileRead(&file, 0, &path_buffer[offset], FS_MAX_PATH - offset, FsReadOption_None, &bytes_read);

            /* Load set defaultconfig */
            if (LoadConfig(path_buffer))
                return;
        }
        /* Fallback to lewd pics config. */
        g_hoster = lewd_pics;
        g_hoster.url = path_buffer;
    }

    bool LoadConfig(char *path) {
        FsFile file;
        if (R_FAILED(fsFsOpenFile(&sdmc, path, FsOpenMode_Read, &file)))
            return false;

        u64 bytes_read;
        fsFileRead(&file, 0, path, FS_MAX_PATH, FsReadOption_None, &bytes_read);
        json j = json::parse(path);

        g_hoster = {
            .url = j["RequestURL"],
            .file_form = j["FileFormName"],
            .scheme = j.contains("URL") ? j["URL"] : "",
        };

        if (j.contains("DestinationType")) {
            std::string support = j["DestinationType"];
            const char *str = support.c_str();
            while (*str) {
                if (std::memcmp(str, "ImageUploader", 13) == 0) {
                    g_hoster.can_img = true;
                    str += 13;
                } else if (std::memcmp(str, "VideoUploader", 13) == 0) {
                    g_hoster.can_mov = true;
                    str += 13;
                } else {
                    str++;
                }
            }
        }

        /* Append parameters to URL. */
        bool first = true;
        for (auto &[k, v] : j["Parameters"].items()) {
            g_hoster.url += first ? '?' : '&';
            g_hoster.url += k + '=';
            g_hoster.url += v;
            first = false;
        }

        /* Append body arguments. */
        for (auto &[k, v] : j["Arguments"].items())
            g_hoster.body.emplace(k, v);

        /* Make custom http header. */
        for (auto &[k, v] : j["Headers"].items())
            g_hoster.header.push_back(k + ": " + v.get<std::string>());

        return true;
    }

    std::string Upload(const CapsAlbumFileId &file_id) {
        std::unique_ptr<char[]> imgBuffer;

        /* Init curl. */
        CURL *curl = curl_easy_init();
        if (curl == nullptr)
            return "curl init failed";

        /* Init mime. */
        curl_mime *mime = curl_mime_init(curl);

        /* Make file part. */
        curl_mimepart *file_part = curl_mime_addpart(mime);
        curl_mime_name(file_part, g_hoster.file_form.c_str());

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

            /* Associate callback. */
            curl_mime_type(file_part, "video/mp4");
            curl_mime_data_cb(file_part, album::MovieReader::GetStreamSize(), album::MovieReader::Read, nullptr, nullptr, nullptr);
        } else {
            curl_mime_free(mime);
            curl_easy_cleanup(curl);

            return "Unsupported file type";
        }

        /* Append body arguments. */
        for (auto &[k, v] : g_hoster.body) {
            /* Append header entry. String will be duped. */
            auto *arg = curl_mime_addpart(mime);
            curl_mime_name(arg, k.c_str());
            curl_mime_data(arg, v.c_str(), CURL_ZERO_TERMINATED);
        }

        /* Make custom http header. */
        curl_slist *chunk = NULL;
        for (const auto &line : g_hoster.header) {
            /* Append header entry. String will be duped. */
            chunk = curl_slist_append(chunk, line.c_str());
        }

        std::string urlresponse = std::string();

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StringWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &urlresponse);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        curl_easy_setopt(curl, CURLOPT_URL, g_hoster.url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        smInitialize();
        CURLcode res = curl_easy_perform(curl);
        smExit();

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        curl_mime_free(mime);
        curl_easy_cleanup(curl);

        album::MovieReader::Close();

        if (res == CURLE_OK) {
            if (http_code == 200) {
                return album::ParseResponse(urlresponse);
            } else {
                return fmt::MakeString("Http Code: %d", http_code);
            }
        } else {
            return curl_easy_strerror(res);
        }
    }

}
