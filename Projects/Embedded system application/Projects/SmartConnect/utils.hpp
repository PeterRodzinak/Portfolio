#pragma once

#include <lvgl.h>
#include <string>
#include <vector>

std::string textFromFile(const std::string & filename);

lv_img_dsc_t loadBMP(const char * fileName);

std::vector<std::string> textVectorFromFile(const std::string & filename);

uint64_t identifyDate(int year, int month, int day, int hour, int minute, int second);