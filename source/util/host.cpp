#include "util/host.hpp"
#include "util/caps.hpp"
#include <curl/curl.h>
#include "util/common.hpp"

size_t MovieRead(char *buffer, size_t size, size_t nitems, void* reader) {
    return ((caps::MovieReader*)reader)->Read(buffer, size * nitems);
}

size_t StringWrite(const char *contents, size_t size, size_t nmemb, std::string *userp) {
	userp->append(contents, size * nmemb);
	return size * nmemb;
}

Hoster g_Hoster;

Hoster::Hoster() {}

Hoster::~Hoster() {}

void Hoster::Initialize(const nlohmann::json& json) {
    this->name = common::GetString(json, "Name", "Lewd");
    this->url = common::GetString(json, "Url", "https://lewd.pics/p/index.php");
    this->regex = common::GetString(json, "Regex", "");
    this->imageMimeName = common::GetString(json, "ImageName", "file");
    this->videoMimeName = common::GetString(json, "VideoName", "file");
}

void Hoster::SetDefault() {
    this->name = "Lewd.pics";
    this->url = "SetDefault()";
}

std::string Hoster::GetName() {
    return this->name;
}

std::string Hoster::GetUrl() {
    return this->url;
}

std::string Hoster::GetRegex() {
    return this->regex;
}

std::string Hoster::uploadEntry(const CapsAlbumEntry& entry) {
    CURL* curl = curl_easy_init();
    curl_mime* mime = curl_mime_init(curl);
    
    caps::MovieReader movieReader(entry);
    void* imageBuffer;
    curl_mimepart* file_part = curl_mime_addpart(mime);
    if (entry.file_id.content == CapsAlbumFileContents_ScreenShot) {
        curl_mime_name(file_part, this->imageMimeName.c_str());
        imageBuffer = malloc(entry.size);
        caps::getFile(entry, imageBuffer);
        curl_mime_data(file_part, (const char*)imageBuffer, entry.size);
    } else {
        curl_mime_name(file_part, this->videoMimeName.c_str());
        curl_mime_data_cb(file_part, entry.size, MovieRead, nullptr, nullptr, &movieReader);
    }

    std::string response;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StringWrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        printf("perform failed with %d\n", rc);
        return "curl error";
    }

    int rcode;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rcode);

    printf("response code: %d\n", rcode);
    printf("urlresponse: %s\n", response.c_str());

    curl_easy_cleanup(curl);
    curl_mime_free(mime);

    return response;
}
