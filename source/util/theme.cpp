#include "util/theme.hpp"
#include "util/common.hpp"
#include <pu/Plutonium>

Theme g_Theme;

Theme::Theme(const nlohmann::json& json) {
    this->color.background = pu::ui::Color::FromHex(common::getString(json, "Background", "#FFFFFF"));
    this->color.focus = pu::ui::Color::FromHex(common::getString(json, "Focus", "#FFFFFF"));
    this->color.text = pu::ui::Color::FromHex(common::getString(json, "Text", "#FFFFFF"));
    this->color.topbar = pu::ui::Color::FromHex(common::getString(json, "Topbar", "#FFFFFF"));
    this->background_path = common::getString(json, "Background Image", "#000000");
    this->image.x = common::getInt(json, "X", 500);
    this->image.y = common::getInt(json, "Y", 500);
    this->image.w = common::getInt(json, "W", 200);
    this->image.h = common::getInt(json, "H", 200);
}
