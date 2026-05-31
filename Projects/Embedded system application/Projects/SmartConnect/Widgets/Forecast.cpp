#include "Forecast.hpp"
#include "../utils.hpp"
#include <ArduinoJson.h>
#include <memory>
#include <RequestBuilder.h>
#include <FFat.h>
#include "../Utilities/HTTPSClient.hpp"



void updateTask(void *params) {
    bool * checker = (bool *) params;
    while (!WiFi.isConnected()) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    File forecastData = FFat.open("", "w", true);

    
    vTaskDelete(NULL);
}

void forecastApp::update() {
    auto dateID = identifyDate(forecastYear, forecastMonth, forecastDay, forecastHour, 0, 0);
    String tmp = String(temperatureForecast[dateID].temperature) + "°C";
    if (forecastHour >= 6 && forecastHour <= 21)
        forecast_icon = loadBMP(forecastPictureTableDay[temperatureForecast[dateID].weather_code].c_str());
    else
        forecast_icon = loadBMP(forecastPictureTableNight[temperatureForecast[dateID].weather_code].c_str());
    lv_obj_invalidate(forecast_icon_image);
    lv_label_set_text(temperatureLabel, tmp.c_str());
    String cityDate = forecastCity + "\n" + String(forecastDay) + "." + String(forecastMonth) + ". " + String(forecastHour) + "h";
    lv_label_set_text(forecastLabel, cityDate.c_str());
}

void forecastApp::setClient(WiFiClientSecure *c) {
    client = c;
}

// updateTask() must be called right after this function, on another core
void forecastApp::changeCityBtn(lv_obj_t * cityButton) {
    String str = lv_list_get_btn_text(cityList, cityButton);
    changeCityStr(str);
}

// updateTask() must be called right after this function, on another core
void forecastApp::changeCityStr(const String &str) {
    if (forecastCity == str)
        return;
    forecastCity = str;
}

bool forecastApp::changeDateTime(int hours) {
    struct tm t;
    t.tm_year = forecastYear - 1900;
    t.tm_mon = forecastMonth - 1;
    t.tm_mday = forecastDay;
    t.tm_hour = forecastHour + hours;
    t.tm_min = 0;
    t.tm_sec = 0;

    time_t forecastTime = mktime(&t);
    if (forecastTime < dateLimitLow || forecastTime > dateLimitHigh) {
        Serial.println("Unable to change");
        return false;
    }

    struct tm timeinfo;
    localtime_r(&forecastTime, &timeinfo);
    forecastYear = timeinfo.tm_year + 1900;
    forecastMonth = timeinfo.tm_mon + 1;
    forecastDay = timeinfo.tm_mday;
    forecastHour = timeinfo.tm_hour;
    return true;
}

std::shared_ptr<forecastApp> forecastApp::getApp(WiFiClientSecure * c) {
    static std::shared_ptr<forecastApp> instance(new forecastApp(c));
    if (c != nullptr)
        instance->setClient(c);
    return instance;
}

void forecastApp::updateData() {
    auto geopos = cityCoordList[forecastCity.c_str()];
    RequestBuilder request(
        "GET",
        "api.open-meteo.com",
        "/v1/forecast"
    );
    request.query.add("latitude", geopos.latitude);
    request.query.add("longitude", geopos.longitude);
    request.query.add("hourly", "temperature_2m,weather_code");

    if (!client->connect("api.open-meteo.com", 443)) {
        Serial.println("Connection failed");
        return;
    }
    Serial.println("Forecast connected.");
    while (client->connected()) {
        String line = client->readStringUntil('\n');
        if (line == "\r") break;
        Serial.println(line);
    }
    Serial.println("Header received.");

    String payload = "";
    while (client->available()) {
        payload += client->readStringUntil('\n');
    }
    Serial.println(payload);

    client->stop();

    JsonDocument jsonResponse;
    deserializeJson(jsonResponse, payload);
    JsonDocument weatherData = jsonResponse["hourly"];
    JsonDocument time = weatherData["time"];
    JsonDocument temperature = weatherData["temperature_2m"];
    JsonDocument weather_code = weatherData["weather_code"];

    for (int i = 0; i < time.size(); i++) {
        String dateString = time[i];
        double temp = temperature[i];
        uint8_t wc = weather_code[i];
        
        int year, month, day, hour;
        std::sscanf(dateString.c_str(), "%d-%d-%dT%d:00", &year, &month, &day, &hour);
        temperatureForecast[identifyDate(year, month, day, hour, 0, 0)] = {temp, wc};
    }
}

void forecastApp::createUI(lv_obj_t * parentTile) {
    forecastTileview = lv_tileview_create(parentTile);
    lv_obj_add_style(forecastTileview, &appBackgroundStyle, LV_PART_MAIN);
    forecastTileMap.emplace("Main_Window", lv_tileview_add_tile(forecastTileview, 0, 0, LV_DIR_RIGHT));
    forecastTileMap.emplace("City_Select", lv_tileview_add_tile(forecastTileview, 1, 0, LV_DIR_LEFT));

    forecastLabel = lv_label_create(forecastTileMap["Main_Window"]);
    lv_label_set_text(forecastLabel, forecastCity.c_str());
    lv_obj_set_style_text_color(forecastLabel, lv_color_make(252, 218, 72), LV_PART_MAIN);
    lv_obj_set_style_text_font(forecastLabel, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_obj_set_pos(forecastLabel, 10, 10);

    forecast_icon_image = lv_img_create(forecastTileMap["Main_Window"]);
    forecast_icon = loadBMP("/forecast/clear_day.bmp");
    lv_img_set_src(forecast_icon_image, &forecast_icon);
    lv_obj_set_size(forecast_icon_image, forecast_icon.header.w, forecast_icon.header.h);
    lv_obj_set_pos(forecast_icon_image, 20, (240 - forecast_icon.header.h) / 2);

    temperatureLabel = lv_label_create(forecastTileMap["Main_Window"]);
    lv_label_set_text(temperatureLabel, "???");
    lv_obj_set_style_text_color(temperatureLabel, lv_color_make(255, 255, 255), LV_PART_MAIN);
    lv_obj_set_style_text_font(temperatureLabel, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_pos(temperatureLabel, 120, 100);

    arrow_up_image = loadBMP("/arrow_up.bmp");
    arrow_down_image = loadBMP("/arrow_down.bmp");

    arrow_up_hour = lv_imgbtn_create(forecastTileMap["Main_Window"]);
    lv_imgbtn_set_src(arrow_up_hour, LV_IMGBTN_STATE_RELEASED, &arrow_up_image, &arrow_up_image, &arrow_up_image);
    lv_obj_set_size(arrow_up_hour, arrow_up_image.header.w, arrow_up_image.header.h);
    lv_obj_set_pos(arrow_up_hour, 198, 10);
    lv_obj_add_event_cb(arrow_up_hour, arrowUpPushHour, LV_EVENT_SHORT_CLICKED, NULL);

    arrow_up_day = lv_imgbtn_create(forecastTileMap["Main_Window"]);
    lv_imgbtn_set_src(arrow_up_day, LV_IMGBTN_STATE_RELEASED, &arrow_up_image, &arrow_up_image, &arrow_up_image);
    lv_obj_set_size(arrow_up_day, arrow_up_image.header.w, arrow_up_image.header.h);
    lv_obj_set_pos(arrow_up_day, 160, 10);
    lv_obj_add_event_cb(arrow_up_day, arrowUpPushDay, LV_EVENT_SHORT_CLICKED, NULL);

    arrow_down_hour = lv_imgbtn_create(forecastTileMap["Main_Window"]);
    lv_imgbtn_set_src(arrow_down_hour, LV_IMGBTN_STATE_RELEASED, &arrow_down_image, &arrow_down_image, &arrow_down_image);
    lv_obj_set_size(arrow_down_hour, arrow_down_image.header.w, arrow_down_image.header.h);
    lv_obj_set_pos(arrow_down_hour, 198, 198);
    lv_obj_add_event_cb(arrow_down_hour, arrowDownPushHour, LV_EVENT_SHORT_CLICKED, NULL);

    arrow_down_day = lv_imgbtn_create(forecastTileMap["Main_Window"]);
    lv_imgbtn_set_src(arrow_down_day, LV_IMGBTN_STATE_RELEASED, &arrow_down_image, &arrow_down_image, &arrow_down_image);
    lv_obj_set_size(arrow_down_day, arrow_down_image.header.w, arrow_down_image.header.h);
    lv_obj_set_pos(arrow_down_day, 160, 198);
    lv_obj_add_event_cb(arrow_down_day, arrowDownPushDay, LV_EVENT_SHORT_CLICKED, NULL);

    cityList = lv_list_create(forecastTileMap["City_Select"]);
    lv_obj_add_style(cityList, &appBackgroundStyle, LV_PART_MAIN);
    lv_obj_set_size(cityList, 240, 240);
    for (auto & [city, coords] : cityCoordList) {
        lv_obj_t * btn = lv_list_add_btn(cityList, nullptr, city.c_str());
        lv_obj_add_style(btn, &appBackgroundStyle, LV_PART_MAIN);
        lv_obj_set_style_text_color(btn, lv_color_make(252, 218, 72), LV_PART_MAIN);
        lv_obj_add_event_cb(btn, citySelectHandler, LV_EVENT_CLICKED, nullptr);
    }

    // works in a way that it changes nothing because there is no internet connection at the time
    // will be eventually removed
    updateData();
    update();
}

void forecastApp::citySelectHandler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        auto app = forecastApp::getApp();
        app->changeCityBtn(obj);
        xTaskCreatePinnedToCore(
            updateTask,
            "Forecast Update",
            4000,
            NULL,
            1,
            NULL,
            0
        );
    }
}

forecastApp::forecastApp(WiFiClientSecure * c) {
    time_t now;
    struct tm  timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    forecastYear = timeinfo.tm_year + 1900;
    forecastMonth = timeinfo.tm_mon + 1;
    forecastDay = timeinfo.tm_mday;
    forecastHour = timeinfo.tm_hour = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;
    dateLimitLow = mktime(&timeinfo);
    timeinfo.tm_mday += 7;
    dateLimitHigh = mktime(&timeinfo);

    lv_style_init(&appBackgroundStyle);
    lv_style_set_bg_color(&appBackgroundStyle, lv_color_black());
    client = c;
}

void forecastApp::arrowDownPushDay(lv_event_t *e) {
    auto app = forecastApp::getApp();
    if (app->changeDateTime(-24))
        app->update();
}

void forecastApp::arrowDownPushHour(lv_event_t *e) {
    auto app = forecastApp::getApp();
    if (app->changeDateTime(-1))
        app->update();
}

void forecastApp::arrowUpPushDay(lv_event_t *e) {
    auto app = forecastApp::getApp();
    if (app->changeDateTime(24))
        app->update();
}

void forecastApp::arrowUpPushHour(lv_event_t *e) {
    auto app = forecastApp::getApp();
    if (app->changeDateTime(1))
        app->update();
}

void forecastApp::setActive() {

}

void forecastApp::setBackground() {

}

void forecastApp::setSleeping() {
    
}


