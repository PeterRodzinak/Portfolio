#pragma once

#include "WidgetApp.hpp"
#include <lvgl.h>
#include <memory>

struct pedometerApp : public WidgetApp {    
public:

    static std::shared_ptr<pedometerApp> getApp();

    void updateCounter();

    void createUI(lv_obj_t * parentTile) override;
    virtual void setActive() override;
    virtual void setBackground() override;
    virtual void setSleeping() override;
private:
    
    lv_obj_t * pedometerLabel;
    lv_img_dsc_t pedometer_icon_image;
    lv_obj_t * pedometer_icon;
    bool isActive;

    pedometerApp() = default;
};