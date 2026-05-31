#pragma once

#include <lvgl.h>
#include <memory>
#include "WidgetApp.hpp"
#include "../Utilities/BT.hpp"

class BLEApp : public WidgetApp {
public:

    static std::shared_ptr<BLEApp> getApp();

    void createUI(lv_obj_t * parentTile);
    virtual void setActive() override;
    virtual void setBackground() override;
    virtual void setSleeping() override;
private:

    BLEApp();

    void setup();

    BLECharacteristic * pCharacteristic;

    bool deviceConnected = false;
    bool isAdvertising;
    std::string rxValue;

    lv_obj_t * btImageLabel;
};