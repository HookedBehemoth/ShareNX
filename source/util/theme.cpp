#include "util/theme.hpp"
#include "util/common.hpp"
#include <pu/Plutonium>

Theme::Theme() {}

void Theme::Initialize(const nlohmann::json& json) {
    this->color.background = pu::ui::Color::FromHex(common::GetString(json, "Background", "#6c0000FF"));
    this->color.focus = pu::ui::Color::FromHex(common::GetString(json, "Focus", "#480001FF"));
    this->color.text = pu::ui::Color::FromHex(common::GetString(json, "Text", "#FFFFFFFF"));
    this->color.topbar = pu::ui::Color::FromHex(common::GetString(json, "Topbar", "#170909FF"));
    this->background_path = common::GetString(json, "Background Image", "romfs:/bg.jpg");
    auto& img = json["Image"];
    if (img != nullptr) {
        this->image.path = common::GetString(img, "Path", "romfs:/owo.png");
        this->image.x = common::GetInt(img, "X", 500);
        this->image.y = common::GetInt(img, "Y", 500);
        this->image.w = common::GetInt(img, "W", 200);
        this->image.h = common::GetInt(img, "H", 200);
    }
}
