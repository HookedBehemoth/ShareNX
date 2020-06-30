#pragma once

#include <functional>
#include <map>
#include <string>
#include <switch.h>
#include <vector>

namespace album {

    struct Hoster {
        std::string name;
        bool can_img, can_mov;
        std::string url, file_form;
        std::string scheme;
        std::vector<std::pair<std::string, std::string>> body;
        std::vector<std::string> header;

        std::string Upload(const CapsAlbumFileId &file_id, std::function<bool(size_t, size_t)> cb = [](size_t, size_t) { return true; });
        std::string ParseResponse(const std::string &response);
        void ParseFile(const std::string &path);
    };

    void InitializeHoster();
    void ExitHoster();

    void UpdateHoster();
    std::vector<Hoster> &GetHosterList();

}
