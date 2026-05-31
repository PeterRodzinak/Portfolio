#pragma once

#include "WidgetApp.hpp"
#include <lvgl.h>
#include <memory>

#define MAX_SYNC_ATTEMPT 15

class clockApp : public WidgetApp {
public:

    static std::shared_ptr<clockApp> getApp();

    void update();

    void timeSync();

    void createUI(lv_obj_t * parentTile) override;
    virtual void setActive() override;
    virtual void setBackground() override;
    virtual void setSleeping() override;

    friend void timeSyncCB(lv_timer_t * t);
private:

    clockApp();

    lv_img_dsc_t clock_bg;
    lv_obj_t * clock_bg_image;
    lv_img_dsc_t clock_h;
    lv_obj_t * clock_h_image;
    lv_img_dsc_t clock_m;
    lv_obj_t * clock_m_image;
    lv_img_dsc_t clock_s;
    lv_obj_t * clock_s_image;

    lv_timer_t * ntpTimer;
    lv_timer_t * syncTimer;
    int syncAttemptCounter = 0;

    bool synced;
    bool syncFailed;
    bool isActive;
};