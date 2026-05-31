#pragma once

#include <lvgl.h>

enum AppState {Active, Background, Sleeping};

class WidgetApp {
public:
    AppState getState();

    virtual void createUI(lv_obj_t * parentTile) = 0;
    virtual void setActive() = 0;
    virtual void setBackground() = 0;
    virtual void setSleeping() = 0;

protected:
    void setState(AppState s);

private:
    AppState state;
};