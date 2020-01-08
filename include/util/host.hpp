#pragma once
#include <switch.h>
#include <json.hpp>

extern Hoster g_Hoster;

class Hoster {
public:
    Hoster(const nlohmann::json& json);
    ~Hoster();
    std::string GetName();
    std::string GetUrl();
    std::string GetRegex();
    std::string uploadEntry(const CapsAlbumEntry& entry);
private:
    std::string name;
    std::string url;
    std::string imageMimeName;
    std::string videoMimeName;
    std::string regex;
};
