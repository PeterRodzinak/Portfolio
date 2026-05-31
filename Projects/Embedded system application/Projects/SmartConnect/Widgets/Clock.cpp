#include "Clock.hpp"
#include <LilyGoLib.h>
#include <HTTPClient.h>
#include "../utils.hpp"
#include "../libs/Arduino_JSON/src/Arduino_JSON.h"

#include "../Utilities/HTTPSClient.hpp"

#include <time.h>
#include <sys/time.h>

std::shared_ptr<clockApp> clockApp::getApp() {
    static std::shared_ptr<clockApp> instance(new clockApp());
    return instance;
}

void clockApp::update() {
    auto t = watch.getDateTime();

    if (isActive) {
        lv_img_set_angle(clock_h_image, ((t.hour) * 300 + ((t.minute) * 5)) % 3600);
        lv_img_set_angle(clock_m_image, (t.minute) * 60);
        lv_img_set_angle(clock_s_image, (t.second) * 60);
    }
}

void syncTime(void *pvParameters) {
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    auto t = watch.getDateTime();
    t.year = timeinfo.tm_year + 1900;
    t.month = timeinfo.tm_mon + 1;
    t.day = timeinfo.tm_mday;
    t.hour = timeinfo.tm_hour;
    t.minute = timeinfo.tm_min;
    t.second = timeinfo.tm_sec;
    watch.setDateTime(t);

    vTaskDelete(NULL);
}

void clockApp::createUI(lv_obj_t *parentTile) {
    LV_IMG_DECLARE(clock1_bg);

    clock_bg_image = lv_img_create(parentTile);
    clock_bg = loadBMP("/clock1_bg.bmp");
    lv_img_set_src(clock_bg_image, &clock1_bg);
    lv_obj_set_size(clock_bg_image, clock1_bg.header.w, clock1_bg.header.h);
    lv_obj_set_pos(clock_bg_image, 0, 0);

    clock_h_image = lv_img_create(parentTile);
    clock_h = loadBMP("/clock1_h.bmp");
    lv_img_set_src(clock_h_image, &clock_h);
    lv_obj_set_size(clock_h_image, clock_h.header.w, clock_h.header.h);
    lv_obj_set_pos(clock_h_image, 0, 0);
    lv_img_set_pivot(clock_h_image, clock_h.header.w / 2, clock_h.header.h / 2);

    clock_m_image = lv_img_create(parentTile);
    clock_m = loadBMP("/clock1_m.bmp");
    lv_img_set_src(clock_m_image, &clock_m);
    lv_obj_set_size(clock_m_image, clock_m.header.w, clock_m.header.h);
    lv_obj_set_pos(clock_m_image, 0, 0);
    lv_img_set_pivot(clock_m_image, clock_m.header.w / 2, clock_m.header.h / 2);

    clock_s_image = lv_img_create(parentTile);
    clock_s = loadBMP("/clock1_s.bmp");
    lv_img_set_src(clock_s_image, &clock_s);
    lv_obj_set_size(clock_s_image, clock_s.header.w, clock_s.header.h);
    lv_obj_set_pos(clock_s_image, 0, 0);
    lv_img_set_pivot(clock_s_image, clock_s.header.w / 2, clock_s.header.h / 2);

    lv_timer_create([](lv_timer_t *timer) {
        clockApp::getApp()->update();
    },
    1000, NULL);

    syncTimer = lv_timer_create([](lv_timer_t *timer) {
        if (!WiFi.isConnected())
            return;

        const char* ntpServer = "pool.ntp.org";
        const long  gmtOffset_sec = 7200;           // Adjust for your timezone
        const int   daylightOffset_sec = 0;         // Adjust for daylight saving
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        xTaskCreate(
            syncTime,              // Task function
            "SyncTimeTask",        // Name
            4096,                  // Stack size
            NULL,                  // Parameter
            0,                     // Priority
            NULL                   // Task handle
        );
    },
    10000, NULL);
}

void clockApp::setActive() {
    isActive = true;
}

void clockApp::setBackground() {
    isActive = false;
}

void clockApp::setSleeping() {
    isActive = false;
}

clockApp::clockApp() : synced(false) {}
