#include "SmartHomeRemote.hpp"
#include <HTTPClient.h>

#include "../Utilities/HTTPSClient.hpp"

std::shared_ptr<smartHomeApp> smartHomeApp::getApp() {
    static std::shared_ptr<smartHomeApp> instance(new smartHomeApp());
    return instance;
}

void changeColor() {
    auto App = smartHomeApp::getApp();

    std::string json_body("{");
    json_body.append("\"deviceid\": \"10014a4322\",");
    json_body.append("\"data\": {");
    json_body.append("\"ltype\": \"color\",");
    json_body.append("\"color\": {");
    json_body.append("\"br\": ").append(std::to_string(App->brightness)).append(", ");
    json_body.append("\"r\": ").append(std::to_string(App->red)).append(", ");
    json_body.append("\"g\": ").append(std::to_string(App->green)).append(", ");
    json_body.append("\"b\": ").append(std::to_string(App->blue)).append("}");
    json_body.append("}}");

    HTTPClient http;

    http.begin(App->serverName + App->colorTarget);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(json_body.c_str());

    if (httpResponseCode > 0) {
        String payload = http.getString();
        Serial.println("HTTP Response code: " + String(httpResponseCode));
        Serial.println("Response payload: " + payload);
    } else {
        Serial.println("Error on HTTP request: " + String(httpResponseCode));
    }

    http.end();
}

void redSliderMove(lv_event_t *e) {
    auto App = smartHomeApp::getApp();
    App->red = lv_slider_get_value(App->sliderRed);
    changeColor();
}

void greenSliderMove(lv_event_t *e) {
    auto App = smartHomeApp::getApp();
    App->green = lv_slider_get_value(App->sliderGreen);
    changeColor();
}

void blueSliderMove(lv_event_t *e) {
    auto App = smartHomeApp::getApp();
    App->blue = lv_slider_get_value(App->sliderBlue);
    changeColor();
}

void brightnessSliderMove(lv_event_t *e) {
    auto App = smartHomeApp::getApp();
    App->brightness = lv_slider_get_value(App->sliderBrightness);
    changeColor();
}

void click(lv_event_t *e) {
    auto App = smartHomeApp::getApp();

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        std::string json_body("{");
        json_body.append("\"deviceid\": \"10014a4322\",");
        json_body.append("\"data\": {");
        json_body.append(App->lightbulbState ? R"("switch": "off"})" : R"("switch": "on"})");
        json_body.append("}");

        HTTPClient http;

        http.begin(App->serverName + App->switchTarget);
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST(json_body.c_str());

        if (httpResponseCode > 0) {
            String payload = http.getString();
            Serial.println("HTTP Response code: " + String(httpResponseCode));
            Serial.println("Response payload: " + payload);
            App->lightbulbState = !App->lightbulbState;
            lv_label_set_text_fmt(lv_obj_get_child(btn, 0), "%s", App->lightbulbState ? "Turn off" : "Turn on");
        } else {
            Serial.println("Error on HTTP request: " + String(httpResponseCode));
        }

        http.end();
    }
}

void smartHomeApp::createUI(lv_obj_t *parentTile)
{
    switchButton = lv_btn_create(parentTile);
    lv_obj_set_pos(switchButton, 60, 10);
    lv_obj_set_size(switchButton, 120, 50);
    lv_obj_add_event_cb(switchButton, click, LV_EVENT_ALL, NULL);

    lv_obj_t *buttonLabel = lv_label_create(switchButton);
    lv_label_set_text(buttonLabel, "Turn on");
    lv_obj_center(buttonLabel);

    sliderRed = lv_slider_create(parentTile);
    lv_obj_set_pos(sliderRed, 20, 70);
    lv_obj_set_width(sliderRed, 150);
    lv_slider_set_range(sliderRed, 0, 255);
    lv_slider_set_value(sliderRed, red, LV_ANIM_OFF);
    lv_obj_add_event_cb(sliderRed, redSliderMove, LV_EVENT_RELEASED, NULL);

    sliderGreen = lv_slider_create(parentTile);
    lv_obj_set_pos(sliderGreen, 20, 120);
    lv_obj_set_width(sliderGreen, 150);
    lv_slider_set_range(sliderGreen, 0, 255);
    lv_slider_set_value(sliderGreen, green, LV_ANIM_OFF);
    lv_obj_add_event_cb(sliderGreen, greenSliderMove, LV_EVENT_RELEASED, NULL);

    sliderBlue = lv_slider_create(parentTile);
    lv_obj_set_pos(sliderBlue, 20, 170);
    lv_obj_set_width(sliderBlue, 150);
    lv_slider_set_range(sliderBlue, 0, 255);
    lv_slider_set_value(sliderBlue, blue, LV_ANIM_OFF);
    lv_obj_add_event_cb(sliderBlue, blueSliderMove, LV_EVENT_RELEASED, NULL);

    sliderBrightness = lv_slider_create(parentTile);
    lv_obj_set_pos(sliderBrightness, 20, 220);
    lv_obj_set_width(sliderBrightness, 150);
    lv_slider_set_range(sliderBrightness, 1, 100);
    lv_slider_set_value(sliderBrightness, brightness, LV_ANIM_OFF);
    lv_obj_add_event_cb(sliderBrightness, brightnessSliderMove, LV_EVENT_RELEASED, NULL);
}

void smartHomeApp::setActive() {

}

void smartHomeApp::setBackground() {

}

void smartHomeApp::setSleeping() {
    
}

smartHomeApp::smartHomeApp() {
    red = 0;
    green = 0;
    blue = 0;
    brightness = 100;
    lightbulbState = false;
}
