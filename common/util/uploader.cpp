#include "uploader.hpp"

#include "translation/translation.hpp"
#include "fmt.hpp"
#include "result.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <json.hpp>
#include <memory>
#include <switch.h>

using json = nlohmann::json;

namespace upload {

    namespace {

        constexpr const char *ImgurAuthUrl = "https://api.imgur.com/oauth2/authorize?client_id=" IMGUR_CLIENT_ID "&response_type=token&state=auth";
        constexpr const char *ImgurAuthCallback = "http://localhost";
        constexpr const char *ImgurKeywords[3] = {"token_type=", "access_token=", "account_username="};
        constexpr size_t ImgurKeywordCount = sizeof(ImgurKeywords) / sizeof(const char *);

        constexpr const char *HosterConfigPath = "sdmc:/config/ShareNX/";

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
                        url++;
                        break;
                    }
                }
                url++;
            }
        }

    }

    std::vector<std::string> GetHosterNameList() {
        std::vector<std::string> result;
        for (auto &entry : std::filesystem::directory_iterator(HosterConfigPath)) {
            result.push_back(entry.path().filename());
        }
        return result;
    }

    std::string GenerateImgurConfig() {
        /* We can only use the web browser as an application. */
        THROW_UNLESS(appletGetAppletType() == AppletType_Application, WEB_APPLET_MODE);

        auto config = std::make_unique<WebCommonConfig>();
        R_THROW(webPageCreate(config.get(), ImgurAuthUrl));
        R_THROW(webConfigSetCallbackUrl(config.get(), ImgurAuthCallback));

        auto reply = std::make_unique<WebCommonReply>();
        R_THROW(webConfigShow(config.get(), reply.get()));

        WebExitReason exit_reason;
        R_THROW(webReplyGetExitReason(reply.get(), &exit_reason));

        THROW_UNLESS(exit_reason == WebExitReason_LastUrl, WEB_EXIT_REASON);

        char last_url[1000];
        size_t length;
        R_THROW(webReplyGetLastUrl(reply.get(), last_url, sizeof(last_url), &length));

        /* Parse response URL. */
        const char *results[ImgurKeywordCount] = {};
        GetToken(last_url, ImgurKeywordCount, ImgurKeywords, results);

        /* Validate parsed values. */
        for (size_t i = 0; i < ImgurKeywordCount; i++) {
            THROW_UNLESS(results[i] != nullptr, TOKEN_MISSING);
        }

        json j = json::object(
            {
                {"Version", "12.4.1"},
                {"DestinationType", "ImageUploader"},
                {"RequestMethod", "POST"},
                {"RequestURL", "https://api.imgur.com/3/upload"},
                {"Body", "MultipartFormData"},
                {"FileFormName", "image"},
                {"Headers", {{"Authorization", fmt::MakeString("Bearer %s", results[1])}}},
                {"URL", "$json:data.link$"},
            });

        std::ofstream ofs(fmt::MakeString("%simgur_%s.sxcu", HosterConfigPath, results[2]));
        ofs << j.dump(1, '\t');

        return results[2];
    }

}
