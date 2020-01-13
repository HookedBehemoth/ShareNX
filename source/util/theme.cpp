#include "util/theme.hpp"
#include "util/common.hpp"
#include <pu/Plutonium>

Theme::Theme() {}

void Theme::Initialize(const nlohmann::json& json, std::string name) {
    this->name = name;
    this->color.background = pu::ui::Color::FromHex(common::GetString(json, "Background", "#6c0000FF"));
    this->color.focus = pu::ui::Color::FromHex(common::GetString(json, "Focus", "#480001FF"));
    this->color.text = pu::ui::Color::FromHex(common::GetString(json, "Text", "#FFFFFFFF"));
    this->color.topbar = pu::ui::Color::FromHex(common::GetString(json, "Topbar", "#170909FF"));
    this->background_path = common::GetString(json, "Background Image", "romfs:/bg.jpg");
    if (!json.contains("Image"))
        return;
    auto& img = json["Image"];
    if (!img.is_object())
        return;
    this->image.path = common::GetString(img, "Path", "romfs:/owo.png");
    this->image.x = common::GetInt(img, "X", 500);
    this->image.y = common::GetInt(img, "Y", 500);
    this->image.w = common::GetInt(img, "W", 200);
    this->image.h = common::GetInt(img, "H", 200);
}

void Theme::SetDefault() {
    this->name = "";
    this->color.background = pu::ui::Color(0x6c, 0x00, 0x00, 0xFF);
    this->color.focus = pu::ui::Color(0x48, 0x00, 0x01, 0xFF);
    this->color.text = pu::ui::Color(0xFF, 0xFF, 0xFF, 0xFF);
    this->color.topbar = pu::ui::Color(0x17, 0x09, 0x09, 0xFF);
    this->background_path = "romfs:/bg.jpg";
    this->image.path = "romfs:/owo.png";
    this->image.x = 975;
    this->image.y = 240;
    this->image.w = 292;
    this->image.h = 480;
}
