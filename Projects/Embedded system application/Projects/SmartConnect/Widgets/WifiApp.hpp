#pragma once

#include "WidgetApp.hpp"
#include <string>
#include <memory>
#include <lvgl.h>
#include <WString.h>
#include <unordered_map>
#include <HTTPClient.h>

struct wifiApp : public WidgetApp {
public:
    static std::shared_ptr<wifiApp> getApp();

    void createUI(lv_obj_t * parentTile) override;
    virtual void setActive() override;
    virtual void setBackground() override;
    virtual void setSleeping() override;

    friend void WiFiScanDone(WiFiEvent_t event, WiFiEventInfo_t info);

    friend void wifiButtonClick(lv_event_t *e);

    friend void connectButtonClick(lv_event_t * e);

    friend void exitButtonClick(lv_event_t * e);

    friend void disconnectButtonClick(lv_event_t *e);

    friend void forgetButtonClick(lv_event_t *e);

    friend void passwordAreaClicked(lv_event_t * e);

    friend void passwordKBSubmit(lv_event_t * e);
private:
    wifiApp();

    bool wifiScanEnable;
    bool wifiIsConnecting = false;
    bool wifiAutoconnect = true;
    String activeSSID = "";
    String processedSSID = "";
    std::unordered_map<std::string, std::string> wifiPasswordTable;
    lv_obj_t * mainTile;

    lv_style_t appBackgroundStyle;
    lv_obj_t * wifiList;
    lv_obj_t * wifiCredentialContext;
    lv_obj_t * connectButton;
    lv_obj_t * exitButton;
    lv_obj_t * forgetButton;
    lv_obj_t * disconnectButton;
    lv_obj_t * wifiNameLabel;
    lv_obj_t * wifiPasswordTextArea;
    lv_obj_t * passwordWritingContext;
    lv_obj_t * passwordKeyboard;
    lv_obj_t * passwordTextArea;
    lv_obj_t * connectingSpinner;

    lv_timer_t * wifiScanTimer;

    String autoconnectSSID;
    String autoconnectPass;
};