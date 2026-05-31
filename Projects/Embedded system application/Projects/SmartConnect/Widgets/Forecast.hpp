#pragma once

#include "WidgetApp.hpp"
#include <unordered_map>
#include <lvgl.h>
#include <string>
#include <WString.h>
#include <WiFiClientSecure.h>
#include <map>
#include <memory>

struct Weather {
    double temperature;
    uint8_t weather_code;
};

struct GeoPosition {
    String latitude;
    String longitude;
};

struct forecastApp : public WidgetApp {
public:
    static std::shared_ptr<forecastApp> getApp(WiFiClientSecure * c = nullptr);

    void updateData();

    void update();

    friend void updateTask(void * params);

    void setClient(WiFiClientSecure * c);

    void changeCityBtn(lv_obj_t * cityButton);

    void changeCityStr(const String & str);

    bool changeDateTime(int hours);

    void createUI(lv_obj_t * parentTile) override;
    virtual void setActive() override;
    virtual void setBackground() override;
    virtual void setSleeping() override;

    static void citySelectHandler(lv_event_t *e);

    static void arrowUpPushHour(lv_event_t *e);

    static void arrowDownPushHour(lv_event_t *e);

    static void arrowUpPushDay(lv_event_t *e);

    static void arrowDownPushDay(lv_event_t *e);
    
private:
    forecastApp(WiFiClientSecure * c);

    WiFiClientSecure * client;

    lv_style_t appBackgroundStyle;
    lv_obj_t * forecastTileview;
    std::unordered_map<std::string, lv_obj_t *> forecastTileMap;
    lv_obj_t * forecastLabel;
    lv_obj_t * temperatureLabel;
    lv_img_dsc_t forecast_icon;
    lv_obj_t * forecast_icon_image;
    lv_img_dsc_t arrow_up_image;
    lv_img_dsc_t arrow_down_image;
    lv_obj_t * arrow_up_hour;
    lv_obj_t * arrow_up_day;
    lv_obj_t * arrow_down_hour;
    lv_obj_t * arrow_down_day;
    lv_obj_t * cityList;

    time_t dateLimitLow;
    time_t dateLimitHigh;
    int forecastReferenceDay;
    int forecastYear, forecastMonth, forecastDay, forecastHour;
    String forecastCity = "Praha";

    std::map<std::string, GeoPosition> cityCoordList = {
        {"Praha", {"52.52", "13.41"}},
        {"Bratislava", {"48.14", "17.10"}},
        {"Banska Bystrica", {"48.73", "19.14"}},
        {"Brno", {"49.19", "16.60"}},
        {"Zilina", {"49.22", "18.73"}},
        {"Nitra", {"48.30", "18.08"}},
        {"Trnava", {"48.37", "17.58"}},
        {"Trencin", {"48.89", "18.04"}},
        {"Kosice", {"48.71", "21.26"}},
        {"Presov", {"49.00", "21.23"}},
    };
    std::unordered_map<time_t, Weather> temperatureForecast;
    std::unordered_map<unsigned int, String> forecastPictureTableDay = {
        {0, "/forecast/clear_day.bmp"},
        {1, "/forecast/clear_day.bmp"},
        {2, "/forecast/half_cloudy.bmp"},
        {3, "/forecast/cloudy.bmp"},
        {45, "/forecast/cloud.bmp"},
        {48, "/forecast/cloud.bmp"},
        {51, "/forecast/rain_sunny.bmp"},
        {53, "/forecast/rain_sunny.bmp"},
        {55, "/forecast/rain_sunny.bmp"},
        {56, "/forecast/rain_sunny.bmp"},
        {57, "/forecast/rain_sunny.bmp"},
        {61, "/forecast/rain_sunny.bmp"},
        {63, "/forecast/rain_sunny.bmp"},
        {65, "/forecast/rain_sunny.bmp"},
        {71, "/forecast/snow_sunny.bmp"},
        {73, "/forecast/snow_sunny.bmp"},
        {75, "/forecast/snow_sunny.bmp"},
        {77, "/forecast/snow_sunny.bmp"},
        {80, "/forecast/rain_sunny.bmp"},
        {81, "/forecast/rain_sunny.bmp"},
        {82, "/forecast/rain_sunny.bmp"},
        {85, "/forecast/snow_sunny.bmp"},
        {86, "/forecast/snow_sunny.bmp"},
        {95, "/forecast/storm.bmp"},
        {96, "/forecast/hail.bmp"},
        {99, "/forecast/hail.bmp"}
    };

    std::unordered_map<unsigned int, String> forecastPictureTableNight = {
        {0, "/forecast/clear_night.bmp"},
        {1, "/forecast/clear_night.bmp"},
        {2, "/forecast/half_cloudy_night.bmp"},
        {3, "/forecast/cloudy_night.bmp"},
        {45, "/forecast/cloud.bmp"},
        {48, "/forecast/cloud.bmp"},
        {51, "/forecast/rain_night.bmp"},
        {53, "/forecast/rain_night.bmp"},
        {55, "/forecast/rain_night.bmp"},
        {56, "/forecast/rain_night.bmp"},
        {57, "/forecast/rain_night.bmp"},
        {61, "/forecast/rain_night.bmp"},
        {63, "/forecast/rain_night.bmp"},
        {65, "/forecast/rain_night.bmp"},
        {71, "/forecast/snow_night.bmp"},
        {73, "/forecast/snow_night.bmp"},
        {75, "/forecast/snow_night.bmp"},
        {77, "/forecast/snow_night.bmp"},
        {80, "/forecast/rain_night.bmp"},
        {81, "/forecast/rain_night.bmp"},
        {82, "/forecast/rain_night.bmp"},
        {85, "/forecast/snow_night.bmp"},
        {86, "/forecast/snow_night.bmp"},
        {95, "/forecast/storm.bmp"},
        {96, "/forecast/hail.bmp"},
        {99, "/forecast/hail.bmp"}
    };
};