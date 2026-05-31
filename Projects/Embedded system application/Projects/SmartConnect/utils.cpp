#include "utils.hpp"
#include <cstdlib>
#include "lvgl.h"
#include "FS.h"
#include "FFat.h"

uint64_t identifyDate(int year, int month, int day, int hour, int minute, int second) {
    struct tm date = {0};

    date.tm_year = year - 1900;
    date.tm_mon = month - 1;
    date.tm_mday = day;
    date.tm_hour = hour;
    date.tm_min = minute;
    date.tm_sec = second;

    return mktime(&date);
}

void dummyRead(fs::File & file, size_t byteCount) {
    uint8_t * dummyBuffer = new uint8_t[byteCount];
    if (byteCount != 0)
        file.read(dummyBuffer, byteCount);
    delete [] dummyBuffer;
}

std::string textFromFile(const std::string & filename) {
    auto imageFile = FFat.open(filename.c_str(), FILE_READ);

    uint8_t buf[257] = {0};
    uint16_t readBytes = 0;
    std::string content;
    while (readBytes = imageFile.read(buf, 256)) {
        if (readBytes != 256) {
            for (int i = 0; i + readBytes < 256; i++) {
                buf[readBytes + i] = 0;
            }
        }
        
        content.append((char *) buf);
    }
    return content;
}

std::vector<std::string> textVectorFromFile(const std::string & filename){
    auto imageFile = FFat.open(filename.c_str(), FILE_READ);

    std::vector<std::string> textVec;
    std::string line;
    while (imageFile.peek() != -1) {
        line = imageFile.readStringUntil('\n').c_str();
        if (line.back() == '\r')
            line.back() = 0;
        textVec.push_back(std::move(line));
        line.clear();
    }

    return textVec;
}

lv_img_dsc_t loadBMP(const char * fileName) {
    auto imageFile = FFat.open(fileName, FILE_READ);

    uint8_t signature[2] = {0};
    imageFile.read(signature, 2);
    if (signature[0] != 'B' || signature[1] != 'M') {
        //Serial.printf("File %s was not a BMP file, signature was %d%d\n\r", fileName, signature[0], signature[1]);
        return {};
    } else {
        //Serial.printf("****** Loading %s ******\n\r", fileName);
    }

    unsigned int imageSize;
    imageFile.read((uint8_t *)&imageSize, 4);
    //Serial.printf("imageSize = %u\n\r", imageSize);
    
    dummyRead(imageFile, 4);
    unsigned int offset;
    imageFile.read((uint8_t *)&offset, 4);
    //Serial.printf("offset = %u\n\r", offset);
    offset -= 2 + 4 + 4 + 4;

    uint32_t headerSize;
    imageFile.read((uint8_t *)&headerSize, 4);
    //Serial.printf("headerSize = %u\n\r", headerSize);

    uint32_t width, height;
    imageFile.read((uint8_t *)&width, 4);
    imageFile.read((uint8_t *)&height, 4);
    //Serial.printf("width = %u\n\r", width);
    //Serial.printf("height = %u\n\r", height);
    offset -= 12;

    uint16_t planes;
    uint16_t bitCount;
    uint32_t compression;
    uint32_t sizeImage;
    uint32_t xPelsPerMeter;
    uint32_t YPelsPerMeter;
    uint32_t clrUsed;
    uint32_t clrImportant;

    imageFile.read((uint8_t *)&planes, 2);
    imageFile.read((uint8_t *)&bitCount, 2);
    imageFile.read((uint8_t *)&compression, 4);
    imageFile.read((uint8_t *)&sizeImage, 4);
    imageFile.read((uint8_t *)&xPelsPerMeter, 4);
    imageFile.read((uint8_t *)&YPelsPerMeter, 4);
    imageFile.read((uint8_t *)&clrUsed, 4);
    imageFile.read((uint8_t *)&clrImportant, 4);    
    offset -= 28;

    /*Serial.printf("planes = %u\n\r", planes);
    Serial.printf("bitCount = %u\n\r", bitCount);
    Serial.printf("compression = %u\n\r", compression);
    Serial.printf("sizeImage = %u\n\r", sizeImage);
    Serial.printf("xPelsPerMeter = %u\n\r", xPelsPerMeter);
    Serial.printf("YPelsPerMeter = %u\n\r", YPelsPerMeter);
    Serial.printf("clrUsed = %u\n\r", clrUsed);
    Serial.printf("clrImportant = %u\n\r", clrImportant);
    Serial.printf("Remaining offset = %u\n\r", offset);*/
    if (height > 240) {
        height = (sizeImage / (bitCount / 8)) / width;
        //Serial.printf("New height had to be calculated: %u\n\r", height);
    }

    //Serial.println();

    struct pixel {
        uint8_t alpha;
        uint16_t colorData;
    };

    
    dummyRead(imageFile, offset);


    unsigned int pictureDataSize = width*height;

    uint8_t color[3] = {0};
    uint8_t byteCount = bitCount / 8;
    pixel p;

    uint8_t * imageBuffer;
    uint8_t * rowData;
    try {
        imageBuffer = new uint8_t[pictureDataSize * 3];
        rowData = new uint8_t[byteCount * width];
    } catch (...) {
        return {};
    }

    for (int i = height - 1; i >= 0; i--) {
        imageFile.read(rowData, byteCount * width);
        for (int j = 0; j < width * 3; j += 3) {
            color[0] = rowData[(j / 3) * byteCount] >> 3;
            color[1] = rowData[(j / 3) * byteCount + 1] >> 2;
            color[2] = rowData[(j / 3) * byteCount + 2] >> 3;

            p.colorData = (uint16_t)color[0] | ((uint16_t)color[1] << 5) | ((uint16_t)color[2] << 11);
            
            if (byteCount == 4) {
                p.alpha = rowData[(j / 3) * byteCount + 3];
            } else {
                p.alpha = 0xFF;
            }

            imageBuffer[i * width * 3 + j] = p.colorData;
            imageBuffer[i * width * 3 + j + 1] = p.colorData >> 8;
            imageBuffer[i * width * 3 + j + 2] = p.alpha;
        }
    }


    delete [] rowData;
    imageFile.close();
    
    lv_img_dsc_t resultImage;
    resultImage.header.always_zero = 0;
    resultImage.header.w = width;
    resultImage.header.h = height;
    resultImage.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    resultImage.data_size = pictureDataSize * 3;
    resultImage.data = imageBuffer;

    //Serial.printf("Loading of image %s has been successfully completed\n\r\n\r", fileName);

    return resultImage;
}