#pragma once

#include <LilyGoLib.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-123546789abc"

#define CHARACTERISTIC_UUID "abcd1234-ab12-cd34-ef56-21345678abcd"

class MyCallbacks : public BLECharacteristicCallbacks {
    HWCDC & serial;

    void onWrite(BLECharacteristic * pCharacreristic);
public:
    MyCallbacks(HWCDC & s) : serial(s) {}
};