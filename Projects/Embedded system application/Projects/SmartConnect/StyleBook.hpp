#pragma once

#include <string>
#include <unordered_map>
#include "lvgl.h"
#include "WString.h"

class Stylebook {
public:
    Stylebook();

    const lv_style_t & operator [] (const char * tag) const;

    const lv_style_t & operator [] (const std::string & tag) const;

    const lv_style_t & operator [] (const String & tag) const;
private:
    std::unordered_map<std::string, lv_style_t> styleMap;
};

Stylebook::Stylebook() {
    lv_style_t backgroundStyle;
    lv_style_init(&backgroundStyle);
    lv_style_set_bg_color(&backgroundStyle, lv_color_black());
    styleMap["background"] = backgroundStyle;

    
}