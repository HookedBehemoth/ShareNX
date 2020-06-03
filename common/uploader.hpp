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
        std::map<std::string, std::string> body;
        std::vector<std::string> header;

        std::string Upload(const CapsAlbumFileId &file_id, std::function<void(double, double)> cb = [](double, double) {});
        std::string ParseResponse(const std::string &response);
        void ParseFile(const std::string &path);
    };

    void Initialize();
    void Exit();

    void Update();
    std::vector<Hoster> &GetHosterList();

}
