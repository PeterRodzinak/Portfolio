#pragma once

#include "WidgetApp.hpp"
#include <lvgl.h>
#include <memory>
#include <Arduino.h>

class smartHomeApp : public WidgetApp {
public:
    static std::shared_ptr<smartHomeApp> getApp();

    void createUI(lv_obj_t * parentTile) override;
    virtual void setActive() override;
    virtual void setBackground() override;
    virtual void setSleeping() override;

    friend void changeColor();

    friend void redSliderMove(lv_event_t *e);

    friend void greenSliderMove(lv_event_t *e);

    friend void blueSliderMove(lv_event_t *e);

    friend void brightnessSliderMove(lv_event_t *e);

    friend void click(lv_event_t *e);
private:
    smartHomeApp();

    uint8_t red, green, blue, brightness;
    bool lightbulbState;
    String serverName = "http://192.168.1.117:8081";
    String switchTarget = "/zeroconf/switch";
    String colorTarget = "/zeroconf/dimmable";
    lv_obj_t * label;
    lv_obj_t * switchButton;
    lv_obj_t * sliderRed;
    lv_obj_t * sliderGreen;
    lv_obj_t * sliderBlue;
    lv_obj_t * sliderBrightness;
};