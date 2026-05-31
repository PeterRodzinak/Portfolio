#include "Stopwatch.hpp"

#include <LilyGoLib.h>

#include <sstream>
#include <iomanip>

void addBtnLabel(lv_obj_t * parent, const char * text) {
    lv_obj_t * label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_center(label);
}

std::shared_ptr<StopwatchApp> StopwatchApp::getApp() {
    static std::shared_ptr<StopwatchApp> instance(new StopwatchApp());
    return instance;
}

void StopwatchApp::refreshSetupCounter() {
    uint16_t minutes = setupCounter / 60;
    uint16_t seconds = setupCounter % 60;

    std::stringstream iss;
    iss << std::setfill('0') << std::setw(2) << minutes << ":" << std::setfill('0') << std::setw(2) << seconds; 
    lv_label_set_text(counterLabel, iss.str().c_str());
    lv_obj_align(counterLabel, LV_ALIGN_TOP_MID, 0, 10);
}

void StopwatchApp::printMainCountdown() {
    uint16_t minutes = curTime / 60;
    uint16_t seconds = curTime % 60;

    std::stringstream iss;
    iss << std::setfill('0') << std::setw(2) << minutes << ":" << std::setfill('0') << std::setw(2) << seconds; 
    lv_label_set_text(mainCounterLbl, iss.str().c_str());
    lv_obj_align(mainCounterLbl, LV_ALIGN_TOP_MID, 0, 50);
}

void StopwatchApp::counterModify(lv_obj_t * buttonPtr) {
    if (buttonPtr == sAdd1btn) {
        setupCounter += 1;
    } else if (buttonPtr == sAdd10btn) {
        setupCounter += 10;
    } else if (buttonPtr == sSub1btn) {
        setupCounter -= (setupCounter >= 1) ? 1 : setupCounter;
    } else if (buttonPtr == sSub10btn) {
        setupCounter -= (setupCounter >= 10) ? 10 : setupCounter;
    } else if (buttonPtr == mAdd1btn) {
        setupCounter += 60;
    } else if (buttonPtr == mAdd10btn) {
        setupCounter += 600;
    } else if (buttonPtr == mSub1btn) {
        setupCounter -= (setupCounter >= 60) ? 60 : setupCounter;
    } else if (buttonPtr == mSub10btn) {
        setupCounter -= (setupCounter >= 600) ? 600 : setupCounter;
    }

    refreshSetupCounter();
}

void StopwatchApp::addPhase() {
    if (setupCounter == 0)
        return;
    phaseVec.emplace_back(setupCounter);
    phaseTableEntries.emplace_back(lv_label_get_text(counterLabel));
    lv_table_set_row_cnt(phaseTable, phaseVec.size());
    for (int i = 0; i < phaseVec.size(); i++) {
        std::string entryText = std::to_string(i + 1);
        entryText += ". phase: ";
        entryText += phaseTableEntries[i];
        lv_table_set_cell_value_fmt(phaseTable, i, 0, entryText.c_str());
    }
    setupCounter = 0;
    refreshSetupCounter();
}

void StopwatchApp::clearSession() {
    phaseVec.clear();
    phaseTableEntries.clear();
    lv_table_set_row_cnt(phaseTable, 0);
    lv_obj_invalidate(phaseTable);
}

void StopwatchApp::progress1s() {
    if (curTime > 0) {
        curTime--;
    } else if (++curPhase < curPhaseVec.size()) {
        watch.setWaveform(0, 16);
        watch.run();
        curTime = phaseVec[curPhase];
    } else {
        watch.setWaveform(0, 15);
        watch.run();
        return;
    }

    if (isActive)
        printMainCountdown();
}

void StopwatchApp::manageCountdownClick() {
    if (phaseVec.size() == 0)
        return;

    if (!countdown) {
        countdown = true;
        curPhaseVec = phaseVec;
        curPhase = 0;
        curTime = curPhaseVec[curPhase];
        lv_label_set_text(startSessionBtnLbl, "Stop");
        lv_obj_center(startSessionBtnLbl);
        if (isActive)
            printMainCountdown();
        
        mainTimer = lv_timer_create([](lv_timer_t * timer) {
            auto app = StopwatchApp::getApp();
            app->progress1s();
        }, 1000, nullptr);
    } else {
        countdown = false;
        lv_label_set_text(mainCounterLbl, "00:00");
        lv_obj_align(mainCounterLbl, LV_ALIGN_TOP_MID, 0, 50);

        lv_label_set_text(startSessionBtnLbl, "Start");
        lv_obj_center(startSessionBtnLbl);
        lv_timer_del(mainTimer);
    }
}

static void addPhaseClick(lv_event_t *e) {
    auto app = StopwatchApp::getApp();
    app->addPhase();
}

static void clearSessionClick(lv_event_t *e) {
    auto app = StopwatchApp::getApp();
    app->clearSession();
}

static void counterModifyClick(lv_event_t *e) {
    auto app = StopwatchApp::getApp();
    app->counterModify(lv_event_get_target(e));
}

static void stopwatchBtnClick(lv_event_t *e) {
    auto app = StopwatchApp::getApp();
    app->manageCountdownClick();
}

void StopwatchApp::createUI(lv_obj_t * parentTile) {
    
    lv_style_init(&appBackgroundStyle);
    lv_style_set_bg_color(&appBackgroundStyle, lv_color_black());
    lv_style_init(&whiteTextStyle);
    lv_style_set_bg_color(&whiteTextStyle, lv_color_white());

    mainTileview = lv_tileview_create(parentTile);
    lv_obj_add_style(mainTileview, &appBackgroundStyle, LV_PART_MAIN);
    tileMap.emplace("Main_Counter", lv_tileview_add_tile(mainTileview, 0, 0, LV_DIR_RIGHT));
    tileMap.emplace("Session_View", lv_tileview_add_tile(mainTileview, 1, 0, LV_DIR_RIGHT | LV_DIR_LEFT));
    tileMap.emplace("Stopwatch_Setup", lv_tileview_add_tile(mainTileview, 2, 0, LV_DIR_LEFT));

    mainCounterLbl = lv_label_create(tileMap["Main_Counter"]);
    lv_obj_set_style_text_font(mainCounterLbl, &lv_font_montserrat_42, LV_PART_MAIN);
    lv_obj_set_style_text_color(mainCounterLbl, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(mainCounterLbl, "00:00");
    lv_obj_align(mainCounterLbl, LV_ALIGN_TOP_MID, 0, 50);

    startSessionBtn = lv_btn_create(tileMap["Main_Counter"]);
    lv_obj_set_size(startSessionBtn, 100, 50);
    lv_obj_align(startSessionBtn, LV_ALIGN_TOP_MID, 0, 150);
    lv_obj_add_event_cb(startSessionBtn, stopwatchBtnClick, LV_EVENT_SHORT_CLICKED, nullptr);

    startSessionBtnLbl = lv_label_create(startSessionBtn);
    lv_label_set_text(startSessionBtnLbl, "Start");
    lv_obj_center(startSessionBtnLbl);
    
    lv_style_init(&sessionTableStyle);
    lv_style_set_bg_opa(&sessionTableStyle, LV_OPA_TRANSP);
    lv_style_set_bg_img_opa(&sessionTableStyle, LV_OPA_TRANSP);
    lv_style_set_line_opa(&sessionTableStyle, LV_OPA_50);
    lv_style_set_border_width(&sessionTableStyle, 1);
    lv_style_set_border_color(&sessionTableStyle, lv_color_make(255,255,255));
    lv_style_set_text_color(&sessionTableStyle, lv_color_make(255,255,255));
    lv_style_set_text_font(&sessionTableStyle, &lv_font_montserrat_16);

    phaseTable = lv_table_create(tileMap["Session_View"]);
    lv_obj_set_size(phaseTable, LV_PCT(100), LV_PCT(100));
    lv_obj_set_scroll_dir(phaseTable, LV_DIR_VER);
    lv_obj_set_size(phaseTable, LV_PCT(100), LV_PCT(100));
    lv_table_set_col_width(phaseTable, 0, lv_disp_get_ver_res(NULL));
    lv_table_set_row_cnt(phaseTable, 1);
    lv_table_set_col_cnt(phaseTable, 1);
    lv_obj_add_style(phaseTable, &sessionTableStyle, LV_PART_MAIN);
    lv_obj_add_style(phaseTable, &sessionTableStyle, LV_PART_ITEMS);
    lv_obj_set_pos(phaseTable, 0, 0);

    counterLabel = lv_label_create(tileMap["Stopwatch_Setup"]);
    lv_obj_set_style_text_color(counterLabel, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(counterLabel, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_label_set_text(counterLabel, "00:00");
    lv_obj_align(counterLabel, LV_ALIGN_TOP_MID, 0, 10);

    lv_style_init(&modifyBtnStyle);
    lv_style_set_bg_color(&modifyBtnStyle, lv_color_make(192, 0, 0));

    sAdd1btn = lv_btn_create(tileMap["Stopwatch_Setup"]);
    lv_obj_set_size(sAdd1btn, 30, 30);
    lv_obj_add_style(sAdd1btn, &modifyBtnStyle, LV_PART_MAIN);
    addBtnLabel(sAdd1btn, "+1");
    lv_obj_add_event_cb(sAdd1btn, counterModifyClick, LV_EVENT_SHORT_CLICKED, nullptr);
    lv_obj_set_pos(sAdd1btn, 15, 85);

    sAdd10btn = lv_btn_create(tileMap["Stopwatch_Setup"]);
    lv_obj_set_size(sAdd10btn, 30, 30);
    lv_obj_add_style(sAdd10btn, &modifyBtnStyle, LV_PART_MAIN);
    addBtnLabel(sAdd10btn, "+10");
    lv_obj_add_event_cb(sAdd10btn, counterModifyClick, LV_EVENT_SHORT_CLICKED, nullptr);
    lv_obj_set_pos(sAdd10btn, 55, 85);

    sSub1btn = lv_btn_create(tileMap["Stopwatch_Setup"]);
    lv_obj_set_size(sSub1btn, 30, 30);
    lv_obj_add_style(sSub1btn, &modifyBtnStyle, LV_PART_MAIN);
    addBtnLabel(sSub1btn, "-1");
    lv_obj_add_event_cb(sSub1btn, counterModifyClick, LV_EVENT_SHORT_CLICKED, nullptr);
    lv_obj_set_pos(sSub1btn, 195, 85);

    sSub10btn = lv_btn_create(tileMap["Stopwatch_Setup"]);
    lv_obj_set_size(sSub10btn, 30, 30);
    lv_obj_add_style(sSub10btn, &modifyBtnStyle, LV_PART_MAIN);
    addBtnLabel(sSub10btn, "-10");
    lv_obj_add_event_cb(sSub10btn, counterModifyClick, LV_EVENT_SHORT_CLICKED, nullptr);
    lv_obj_set_pos(sSub10btn, 155, 85);

    mAdd1btn = lv_btn_create(tileMap["Stopwatch_Setup"]);
    lv_obj_set_size(mAdd1btn, 30, 30);
    lv_obj_add_style(mAdd1btn, &modifyBtnStyle, LV_PART_MAIN);
    addBtnLabel(mAdd1btn, "+1");
    lv_obj_add_event_cb(mAdd1btn, counterModifyClick, LV_EVENT_SHORT_CLICKED, nullptr);
    lv_obj_set_pos(mAdd1btn, 15, 125);

    mAdd10btn = lv_btn_create(tileMap["Stopwatch_Setup"]);
    lv_obj_set_size(mAdd10btn, 30, 30);
    lv_obj_add_style(mAdd10btn, &modifyBtnStyle, LV_PART_MAIN);
    addBtnLabel(mAdd10btn, "+10");
    lv_obj_add_event_cb(mAdd10btn, counterModifyClick, LV_EVENT_SHORT_CLICKED, nullptr);
    lv_obj_set_pos(mAdd10btn, 55, 125);

    mSub1btn = lv_btn_create(tileMap["Stopwatch_Setup"]);
    lv_obj_set_size(mSub1btn, 30, 30);
    lv_obj_add_style(mSub1btn, &modifyBtnStyle, LV_PART_MAIN);
    addBtnLabel(mSub1btn, "-1");
    lv_obj_add_event_cb(mSub1btn, counterModifyClick, LV_EVENT_SHORT_CLICKED, nullptr);
    lv_obj_set_pos(mSub1btn, 195, 125);

    mSub10btn = lv_btn_create(tileMap["Stopwatch_Setup"]);
    lv_obj_set_size(mSub10btn, 30, 30);
    lv_obj_add_style(mSub10btn, &modifyBtnStyle, LV_PART_MAIN);
    addBtnLabel(mSub10btn, "-10");
    lv_obj_add_event_cb(mSub10btn, counterModifyClick, LV_EVENT_SHORT_CLICKED, nullptr);
    lv_obj_set_pos(mSub10btn, 155, 125);

    lv_obj_t * secLabel = lv_label_create(tileMap["Stopwatch_Setup"]);
    lv_obj_set_style_text_color(secLabel, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(secLabel, "Sec");
    lv_obj_align(secLabel, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t * minLabel = lv_label_create(tileMap["Stopwatch_Setup"]);
    lv_obj_set_style_text_color(minLabel, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(minLabel, "Min");
    lv_obj_align(minLabel, LV_ALIGN_CENTER, 0, 20);

    addPhaseBtn = lv_btn_create(tileMap["Stopwatch_Setup"]);
    lv_obj_set_size(addPhaseBtn, 100, 35);
    addBtnLabel(addPhaseBtn, "Add Phase");
    lv_obj_set_pos(addPhaseBtn, 15, 195);
    lv_obj_add_event_cb(addPhaseBtn, addPhaseClick, LV_EVENT_SHORT_CLICKED, nullptr);

    clearSessionBtn = lv_btn_create(tileMap["Stopwatch_Setup"]);
    lv_obj_set_size(clearSessionBtn, 100, 35);
    addBtnLabel(clearSessionBtn, "Clear session");
    lv_obj_set_pos(clearSessionBtn, 125, 195);
    lv_obj_add_event_cb(clearSessionBtn, clearSessionClick, LV_EVENT_SHORT_CLICKED, nullptr);

    // debug for table to clear itself, a complete joke why it doesn't work otherwise
    setupCounter = 1;
    addPhase();
    clearSession();
}

void StopwatchApp::setActive() {
    isActive = true;
}

void StopwatchApp::setBackground() {
    isActive = false;
}

void StopwatchApp::setSleeping() {
    isActive = false;
}
