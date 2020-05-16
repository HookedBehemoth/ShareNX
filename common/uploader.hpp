#pragma once

#include <switch.h>
#include <string>
#include <vector>

namespace album {

    void Initialize();
    void Exit();

    const std::string &GetHosterName();
    std::vector<std::string> GetHosterNameList();
    void LoadDefault();
    bool LoadConfig(char *path);
    std::string Upload(const CapsAlbumFileId &file_id);

}
