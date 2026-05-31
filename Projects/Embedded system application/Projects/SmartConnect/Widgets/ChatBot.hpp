#pragma once

#include "WidgetApp.hpp"
#include <lvgl.h>
#include <memory>
#include <string>

class ChatBotApp : public WidgetApp {
public:
    static std::shared_ptr<ChatBotApp> getApp();

    void createUI(lv_obj_t * parentTile) override;
    virtual void setActive() override;
    virtual void setBackground() override;
    virtual void setSleeping() override;
private:
    ChatBotApp();

    std::string textFromAudio();

    std::string getResponse(const std::string & userPrompt);

    void getAudio(const std::string & response);
    
    void sayAudio();
};