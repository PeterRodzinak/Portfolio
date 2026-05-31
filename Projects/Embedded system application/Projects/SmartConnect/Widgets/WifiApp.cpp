#include "WifiApp.hpp"

#include "FS.h"
#include "FFat.h"
#include <WiFi.h>

#include <set>
#include <string>

void WiFiScanDone(WiFiEvent_t event, WiFiEventInfo_t info);

std::shared_ptr<wifiApp> wifiApp::getApp() {
    static std::shared_ptr<wifiApp> instance(new wifiApp());
    return instance;
}

wifiApp::wifiApp() {
    WiFi.onEvent(WiFiScanDone, WiFiEvent_t::ARDUINO_EVENT_WIFI_SCAN_DONE);
    WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info) {
        activeSSID = (char *)info.sc_got_ssid_pswd.ssid;
        wifiIsConnecting = false;
        if (lv_obj_is_valid(connectingSpinner))
            lv_obj_del(connectingSpinner);
    }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);

    fs::File passwordFile = FFat.open("/WifiPasswords.txt", FILE_READ, true);
    String line;
    do {
        String ssid, pass;
        ssid = passwordFile.readStringUntil(',');
        pass = passwordFile.readStringUntil('\n');
        if (ssid.isEmpty())
            break;
        
        Serial.println(line);
        wifiPasswordTable[ssid.c_str()] = pass.c_str();
    } while (!line.isEmpty());
    passwordFile.close();

    wifiScanTimer = nullptr;
}

void passwordKBSubmit(lv_event_t * e) {
    auto WifiApp = wifiApp::getApp();
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e);

    if (code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if (code == LV_EVENT_DEFOCUSED) {
        String passwd = lv_textarea_get_text(WifiApp->passwordTextArea);
        lv_textarea_set_text(WifiApp->wifiPasswordTextArea, passwd.c_str());

        lv_obj_invalidate(WifiApp->wifiPasswordTextArea);

        lv_obj_del(WifiApp->passwordWritingContext);
        return;
    }
}

void passwordAreaClicked(lv_event_t * e) {
    auto WifiApp = wifiApp::getApp();
    WifiApp->passwordWritingContext = lv_obj_create(lv_scr_act());
    lv_obj_add_style(WifiApp->passwordWritingContext, &WifiApp->appBackgroundStyle, LV_PART_MAIN);
    lv_obj_set_size(WifiApp->passwordWritingContext, 240, 240);

    WifiApp->passwordKeyboard = lv_keyboard_create(WifiApp->passwordWritingContext);

    WifiApp->passwordTextArea = lv_textarea_create(WifiApp->passwordWritingContext);
    lv_obj_set_pos(WifiApp->passwordTextArea, 5, 10);
    lv_obj_set_size(WifiApp->passwordTextArea, 200, 40);
    lv_obj_add_event_cb(WifiApp->passwordTextArea, passwordKBSubmit, LV_EVENT_ALL, WifiApp->passwordKeyboard);
    lv_textarea_set_text(WifiApp->passwordTextArea, lv_textarea_get_text(WifiApp->wifiPasswordTextArea));

    lv_obj_clear_state(WifiApp->wifiPasswordTextArea, LV_STATE_FOCUSED);
    lv_obj_add_state(WifiApp->passwordTextArea, LV_STATE_FOCUSED);
    lv_group_focus_obj(WifiApp->passwordTextArea);
}

void forgetButtonClick(lv_event_t *e) {
    auto WifiApp = wifiApp::getApp();
    if (WifiApp->activeSSID == WifiApp->processedSSID) {
        WiFi.disconnect(false, false);
        WifiApp->activeSSID.clear();
    }

    WifiApp->wifiPasswordTable.erase(WifiApp->processedSSID.c_str());
    lv_obj_del(WifiApp->wifiCredentialContext);
    WifiApp->processedSSID = "";
}

void disconnectButtonClick(lv_event_t *e) {
    auto WifiApp = wifiApp::getApp();
    WiFi.disconnect(false, false);
    WifiApp->activeSSID.clear();
    WifiApp->wifiAutoconnect = false;
    lv_obj_del(WifiApp->wifiCredentialContext);
}

void exitButtonClick(lv_event_t * e) {
    auto WifiApp = wifiApp::getApp();
    lv_obj_del(WifiApp->wifiCredentialContext);
    WifiApp->wifiScanEnable = true;
}

void connectButtonClick(lv_event_t * e) {
    auto WifiApp = wifiApp::getApp();
    if (WiFi.isConnected())
        WiFi.disconnect();
    String currentWifi = WifiApp->processedSSID;
    String currentWifiPassword;
    
    if (WifiApp->wifiPasswordTable.count(currentWifi.c_str())) {
        currentWifiPassword = WifiApp->wifiPasswordTable[currentWifi.c_str()].c_str();
    } else {
        currentWifiPassword = lv_textarea_get_text(WifiApp->wifiPasswordTextArea);
    }

    WifiApp->wifiScanEnable = true;

    fs::File passwordFile = FFat.open("/WifiPasswords.txt", FILE_APPEND);

    Serial.println(passwordFile.write((uint8_t *)currentWifi.c_str(), currentWifi.length()));
    passwordFile.write((uint8_t *)",", 1);
    Serial.println(passwordFile.write((uint8_t *)currentWifiPassword.c_str(), currentWifiPassword.length()));
    passwordFile.write((uint8_t *)"\n", 1);
    passwordFile.close();

    WifiApp->wifiPasswordTable[currentWifi.c_str()] = currentWifiPassword.c_str();
    lv_obj_del(WifiApp->wifiCredentialContext);
    WifiApp->processedSSID.clear();

    WiFi.begin(currentWifi, currentWifiPassword);
    WifiApp->wifiAutoconnect = true;

    WifiApp->connectingSpinner = lv_spinner_create(WifiApp->mainTile, 1000, 60);
    lv_obj_set_size(WifiApp->connectingSpinner, 100, 100);
    lv_obj_center(WifiApp->connectingSpinner);
}

void wifiButtonClick(lv_event_t *e) {
    auto WifiApp = wifiApp::getApp();
    WifiApp->wifiScanEnable = false;
    
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    WifiApp->processedSSID = lv_list_get_btn_text(WifiApp->wifiList, obj);

    if (code == LV_EVENT_CLICKED) {
        WifiApp->wifiCredentialContext = lv_obj_create(lv_scr_act());
        lv_obj_add_style(WifiApp->wifiCredentialContext, &WifiApp->appBackgroundStyle, LV_PART_MAIN);
        lv_obj_set_size(WifiApp->wifiCredentialContext, 240, 240);

        if (WifiApp->processedSSID != WifiApp->activeSSID) {
            WifiApp->connectButton = lv_btn_create(WifiApp->wifiCredentialContext);
            lv_obj_set_size(WifiApp->connectButton, 80, 40);
            lv_obj_set_pos(WifiApp->connectButton, 135, 170);
            auto label = lv_label_create(WifiApp->connectButton);
            lv_label_set_text(label, "connect");
            lv_obj_center(label);
            lv_obj_add_event_cb(WifiApp->connectButton, connectButtonClick, LV_EVENT_SHORT_CLICKED, nullptr);   
        } else {
            WifiApp->disconnectButton = lv_btn_create(WifiApp->wifiCredentialContext);
            lv_obj_set_size(WifiApp->disconnectButton, 80, 40);
            lv_obj_set_pos(WifiApp->disconnectButton, 135, 170);
            auto label = lv_label_create(WifiApp->disconnectButton);
            lv_label_set_text(label, "disconnect");
            lv_obj_center(label);
            lv_obj_add_event_cb(WifiApp->disconnectButton, disconnectButtonClick, LV_EVENT_SHORT_CLICKED, nullptr);
        }

        if (WifiApp->wifiPasswordTable.count(WifiApp->processedSSID.c_str())) {
            WifiApp->forgetButton = lv_btn_create(WifiApp->wifiCredentialContext);
            lv_obj_set_size(WifiApp->forgetButton, 80, 40);
            lv_obj_set_pos(WifiApp->forgetButton, 10, 50);
            auto label = lv_label_create(WifiApp->forgetButton);
            lv_label_set_text(label, "forget");
            lv_obj_center(label);
            lv_obj_add_event_cb(WifiApp->forgetButton, forgetButtonClick, LV_EVENT_SHORT_CLICKED, NULL);
        } else {
            WifiApp->wifiPasswordTextArea = lv_textarea_create(WifiApp->wifiCredentialContext);
            lv_obj_set_size(WifiApp->wifiPasswordTextArea, 100, 30);
            lv_obj_set_pos(WifiApp->wifiPasswordTextArea, 10, 50);
            lv_obj_add_event_cb(WifiApp->wifiPasswordTextArea, passwordAreaClicked, LV_EVENT_SHORT_CLICKED, NULL);
        }

        WifiApp->exitButton = lv_btn_create(WifiApp->wifiCredentialContext);
        lv_obj_set_size(WifiApp->exitButton, 80, 40);
        lv_obj_set_pos(WifiApp->exitButton, -5, 170);
        auto label = lv_label_create(WifiApp->exitButton);
        lv_label_set_text(label, "exit");
        lv_obj_center(label);
        lv_obj_add_event_cb(WifiApp->exitButton, exitButtonClick, LV_EVENT_SHORT_CLICKED, nullptr);

        WifiApp->wifiNameLabel = lv_label_create(WifiApp->wifiCredentialContext);
        lv_label_set_text(WifiApp->wifiNameLabel, WifiApp->processedSSID.c_str());
        lv_obj_set_pos(WifiApp->wifiNameLabel, 10, 10);
    }
}

void WiFiScanDone(WiFiEvent_t event, WiFiEventInfo_t info) {
    auto WifiApp = wifiApp::getApp();
    if (!WifiApp->wifiScanEnable)
        return;
    lv_obj_del(WifiApp->wifiList);
    
    WifiApp->wifiList = lv_list_create(WifiApp->mainTile);
    lv_obj_add_style(WifiApp->wifiList, &WifiApp->appBackgroundStyle, LV_PART_MAIN);
    lv_obj_set_size(WifiApp->wifiList, 240, 240);

    int16_t counter = WiFi.scanComplete();

    std::set<std::string> SSIDset;
    for (int i = 0; i < counter; ++i) {
        char tmp[26] = {0};
        lv_obj_t * btn = lv_list_add_btn(WifiApp->wifiList, LV_SYMBOL_WIFI, WiFi.SSID(i).c_str());
        SSIDset.emplace(WiFi.SSID(i).c_str());
        lv_obj_add_style(btn, &WifiApp->appBackgroundStyle, LV_PART_MAIN);
        lv_obj_set_style_text_color(btn, lv_color_make(252, 218, 72), LV_PART_MAIN);
        lv_obj_add_event_cb(btn, wifiButtonClick, LV_EVENT_CLICKED, nullptr);
    }

    if (WiFi.isConnected() || WifiApp->wifiIsConnecting || !WifiApp->wifiAutoconnect)
        return;

    WifiApp->wifiIsConnecting = true;
    for (auto & ssid : SSIDset) {
        if (WifiApp->wifiPasswordTable.count(ssid) == 0)
            continue;
        
        WifiApp->autoconnectSSID = ssid.c_str();
        WifiApp->autoconnectPass = WifiApp->wifiPasswordTable[ssid].c_str();
        break;
    }
    WiFi.begin(WifiApp->autoconnectSSID.c_str(), WifiApp->autoconnectPass.c_str());
}

void wifiApp::createUI(lv_obj_t *parentTile) {
    lv_style_init(&appBackgroundStyle);
    lv_style_set_bg_color(&appBackgroundStyle, lv_color_black());

    mainTile = parentTile;
    wifiList = lv_list_create(mainTile);
    lv_obj_add_style(wifiList, &appBackgroundStyle, LV_PART_MAIN);
    lv_obj_set_size(wifiList, 240, 240);

    lv_obj_set_pos(wifiList, 0, 0);        
}

void wifiApp::setActive() {
    if (WiFi.getMode() != WIFI_STA) {
        WiFi.mode(WIFI_STA);
    }
    WiFi.scanNetworks(true);
}

void wifiApp::setBackground() {
    WiFi.scanDelete();
}

void wifiApp::setSleeping() {
    WiFi.scanDelete();
    if (WiFi.isConnected())
        WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(100);
}


