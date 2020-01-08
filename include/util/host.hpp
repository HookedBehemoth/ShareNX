#pragma once
#include <switch.h>
#include <json.hpp>

class Hoster {
public:
    Hoster();
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
