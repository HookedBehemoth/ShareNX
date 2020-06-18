#include "custom_config.hpp"

#include "../translation/translation.hpp"

#include <album.hpp>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <json.hpp>
#include <memory>
#include <switch.h>
#include <sys/select.h>
/* Broken includes */
#include <curl/curl.h>

using json = nlohmann::json;

namespace album {

    namespace {

        size_t StringWrite(const char *contents, size_t size, size_t nmemb, std::string *userp) {
            userp->append(contents, size * nmemb);
            return size * nmemb;
        }

        constexpr const char *ImgurAuthUrl     = "https://api.imgur.com/oauth2/authorize?client_id=" IMGUR_CLIENT_ID "&response_type=token&state=auth";
        constexpr const char *ImgurKeywords[3] = {"token_type=", "access_token=", "account_username="};
        constexpr size_t ImgurKeywordCount     = sizeof(ImgurKeywords) / sizeof(const char *);

        constexpr const char *OAuthCallback = "http://localhost";

        void GetToken(char *url, int count, const char *const *matches, const char **out) {
            int len[count];
            for (int i = 0; i < count; i++)
                len[i] = strlen(matches[i]);

            while (*url) {
                for (int i = 0; i < count; i++) {
                    if (memcmp(url, matches[i], len[i]) == 0) {
                        url += len[i];
                        out[i] = url;
                        while (*url && *url != '&') {
                            url++;
                        }
                        *url = '\0';
                        break;
                    }
                }
                url++;
            }
        }

        std::string GetLastUrl(const char *url) {
            auto config = std::make_unique<WebCommonConfig>();
            R_THROW(webPageCreate(config.get(), url));
            R_THROW(webConfigSetCallbackUrl(config.get(), OAuthCallback));

            AccountUid uid                     = {};
            PselUserSelectionSettings psel_set = {
                .is_skip_enabled                     = false,
                .is_network_service_account_required = false,
                .show_skip_button                    = true,
                .additional_select                   = true,
                .is_unqualified_user_selectable      = true,
            };
            Result rc = pselShowUserSelector(&uid, &psel_set);
            if (R_FAILED(rc)) {
                if (R_VALUE(rc) == MAKERESULT(124, 1))
                    throw CANCELLED;
                else
                    throw rc;
            }

            R_THROW(webConfigSetUid(config.get(), uid));

            auto reply = std::make_unique<WebCommonReply>();
            R_THROW(webConfigShow(config.get(), reply.get()));

            WebExitReason exit_reason;
            R_THROW(webReplyGetExitReason(reply.get(), &exit_reason));

            THROW_UNLESS(exit_reason == WebExitReason_LastUrl, CANCELLED);

            char last_url[1000];
            size_t length;
            R_THROW(webReplyGetLastUrl(reply.get(), last_url, sizeof(last_url), &length));

            return last_url;
        }

    }

    std::string GenerateImgurConfig() {
        /* We can only use the web browser as an application. */
        THROW_UNLESS(appletGetAppletType() == AppletType_Application, WEB_APPLET_MODE);

        std::string last_url = GetLastUrl(ImgurAuthUrl);

        /* Parse response URL. */
        const char *results[ImgurKeywordCount] = {};
        GetToken(last_url.data(), ImgurKeywordCount, ImgurKeywords, results);

        /* Validate parsed values. */
        for (size_t i = 0; i < ImgurKeywordCount; i++) {
            THROW_UNLESS(results[i] != nullptr, TOKEN_MISSING);
        }

        json j = json::object(
            {
                {"Name", fmt::MakeString("imgur %s", results[2])},
                {"Version", "12.4.1"},
                {"DestinationType", "ImageUploader"},
                {"RequestMethod", "POST"},
                {"RequestURL", "https://api.imgur.com/3/upload"},
                {"Body", "MultipartFormData"},
                {"FileFormName", "image"},
                {"Headers", {{"Authorization", fmt::MakeString("Bearer %s", results[1])}}},
                {"URL", "$json:data.link$"},
            });

        std::ofstream ofs(fmt::MakeString("%simgur_%s.sxcu", album::HosterConfigPath, results[2]));
        ofs << j.dump(1, '\t');

        return results[2];
    }

    std::string GenerateElixireConfig() {
        char username[0x20] = {};
        char password[0x20] = {};

        SwkbdConfig kbd;
        swkbdCreate(&kbd, 0);
        swkbdConfigMakePresetUserName(&kbd);
        swkbdConfigSetOkButtonText(&kbd, "Next");
        Result rc = swkbdShow(&kbd, username, sizeof(username));
        if (R_FAILED(rc)) {
            if (R_VALUE(rc) == MAKERESULT(345, 46))
                throw CANCELLED;
            else
                throw rc;
        }
        swkbdClose(&kbd);

        swkbdCreate(&kbd, 0);
        swkbdConfigMakePresetPassword(&kbd);
        swkbdConfigSetOkButtonText(&kbd, "Confirm");
        rc = swkbdShow(&kbd, password, sizeof(password));
        if (R_FAILED(rc)) {
            if (R_VALUE(rc) == MAKERESULT(345, 46))
                throw CANCELLED;
            else
                throw rc;
        }
        swkbdClose(&kbd);

        json credentials = {
            {"user", username},
            {"password", password},
        };

        std::string input = credentials.dump();
        std::string output;

        CURL *curl = curl_easy_init();

        curl_slist *hdr = nullptr;
        hdr             = curl_slist_append(hdr, "Accept: application/json");
        hdr             = curl_slist_append(hdr, "Content-Type: application/json");
        hdr             = curl_slist_append(hdr, "charset: utf-8");

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StringWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdr);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, input.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, "https://elixi.re/api/apikey");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        CURLcode res = curl_easy_perform(curl);

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK)
            return curl_easy_strerror(res);

        json response = json::parse(output);

        if (http_code != 200) {
            std::printf("Http Code: %ld\nmsg: %s\n", http_code, response.dump(4).c_str());
            return response["message"];
        }

        json j = json::object(
            {
                {"Name", fmt::MakeString("elixi.re %s", username)},
                {"Version", "12.4.1"},
                {"DestinationType", "ImageUploader"},
                {"DestinationType", "VideoUploader"},
                {"RequestMethod", "POST"},
                {"RequestURL", "https://elixi.re/api/upload"},
                {"Body", "MultipartFormData"},
                {"FileFormName", "f"},
                {"Headers", {{"Authorization", response["api_key"]}}},
                {"URL", "$json:url$"},
            });

        std::ofstream ofs(fmt::MakeString("%selixi.re_%s.sxcu", album::HosterConfigPath, username));
        ofs << j.dump(1, '\t');

        return username;
    }

}
