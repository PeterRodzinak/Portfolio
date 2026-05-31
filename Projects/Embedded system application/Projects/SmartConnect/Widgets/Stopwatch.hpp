#pragma once

#include "WidgetApp.hpp"
#include <lvgl.h>
#include <unordered_map>
#include <vector>
#include <cinttypes>
#include <string>
#include <memory>

class StopwatchApp : public WidgetApp {
public:
    static std::shared_ptr<StopwatchApp> getApp();

    void counterModify(lv_obj_t * buttonPtr);

    void addPhase();

    void clearSession();

    void manageCountdownClick();

    void progress1s();

    void createUI(lv_obj_t * parentTile) override;
    virtual void setActive() override;
    virtual void setBackground() override;
    virtual void setSleeping() override;
private:
    StopwatchApp() = default;

    lv_obj_t * mainTileview;
    std::unordered_map<std::string, lv_obj_t *> tileMap;

    lv_style_t appBackgroundStyle;
    lv_style_t whiteTextStyle;
    uint16_t mainCounter = 0;
    lv_obj_t * mainCounterLbl;
    lv_obj_t * startSessionBtn;
    lv_obj_t * startSessionBtnLbl;
    lv_timer_t * mainTimer;
    std::vector<uint16_t> curPhaseVec;
    uint16_t curPhase;
    uint16_t curTime;
    bool countdown = false;
    bool isActive;

    lv_style_t sessionTableStyle;
    lv_obj_t * phaseTable;
    std::vector<uint16_t> phaseVec;
    std::vector<std::string> phaseTableEntries;

    uint16_t setupCounter = 0;
    lv_style_t modifyBtnStyle;
    lv_obj_t * counterLabel;
    lv_obj_t * sAdd1btn;
    lv_obj_t * sAdd10btn;
    lv_obj_t * sSub1btn;
    lv_obj_t * sSub10btn;
    lv_obj_t * mAdd1btn;
    lv_obj_t * mAdd10btn;
    lv_obj_t * mSub1btn;
    lv_obj_t * mSub10btn;
    lv_obj_t * addPhaseBtn;
    lv_obj_t * clearSessionBtn;

    void refreshSetupCounter();

    void printMainCountdown();
};