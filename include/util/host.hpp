#pragma once
#include <switch.h>
#include <json.hpp>
#include "ui/UploadLayout.hpp"

struct Mime {
    std::string name, data;
};

class Hoster {
public:
    Hoster();
    ~Hoster();
    void Initialize(const nlohmann::json& json, std::string name);
    void SetDefault();
    std::string GetName();
    std::string GetUrl();
    std::string GetRegex();
    std::string uploadEntry(const CapsAlbumEntry& entry, ui::UploadLayout* cb_data);
private:
    std::string name;
    std::string url;
    std::string imageMimeName;
    std::string videoMimeName;
    std::string regex;
    std::vector<Mime> mimeData;
};
