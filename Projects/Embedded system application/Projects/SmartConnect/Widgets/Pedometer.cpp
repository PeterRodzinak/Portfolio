#include "Pedometer.hpp"
#include "../utils.hpp"

#include <LilyGoLib.h>

std::shared_ptr<pedometerApp> pedometerApp::getApp() {
    static std::shared_ptr<pedometerApp> instance(new pedometerApp());
    return instance;
}

void pedometerApp::updateCounter() {
    uint32_t stepCounter = watch.getPedometerCounter();
    if (isActive)
        lv_label_set_text_fmt(pedometerLabel, "STEP COUNTER: %u", stepCounter);
}

void pedometerApp::createUI(lv_obj_t *parentTile){
    pedometerLabel = lv_label_create(parentTile);
    lv_obj_set_style_text_color(pedometerLabel, lv_color_make(50, 50, 252), LV_PART_MAIN);
    lv_label_set_text(pedometerLabel, "STEP COUNTER: 0");
    lv_obj_set_pos(pedometerLabel, 60, 200);

    pedometer_icon = lv_img_create(parentTile);
    pedometer_icon_image = loadBMP("/running_figure.bmp");
    lv_img_set_src(pedometer_icon, &pedometer_icon_image);
    lv_obj_set_size(pedometer_icon, pedometer_icon_image.header.w,pedometer_icon_image.header.h);
    lv_obj_center(pedometer_icon);
}

void pedometerApp::setActive() {
    isActive = true;
}

void pedometerApp::setBackground() {
    isActive = false;
}

void pedometerApp::setSleeping(){
    isActive = false;
}
