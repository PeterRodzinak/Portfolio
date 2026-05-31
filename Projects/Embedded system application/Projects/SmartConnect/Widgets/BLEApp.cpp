#include "BLEApp.hpp"

std::shared_ptr<BLEApp> BLEApp::getApp() {
    static std::shared_ptr<BLEApp> instance(new BLEApp());
    return instance;
}

void BLEApp::createUI(lv_obj_t *parentTile) {
    btImageLabel = lv_label_create(parentTile);
    lv_obj_set_style_text_color(btImageLabel, lv_color_make(0, 130, 252), LV_PART_MAIN);
    lv_label_set_text(btImageLabel, LV_SYMBOL_BLUETOOTH);
    lv_obj_center(btImageLabel);
}

void BLEApp::setActive() {
    if (!isAdvertising) {
        BLEDevice::startAdvertising();
        isAdvertising = true;
    }
}

void BLEApp::setBackground() {
    if (!isAdvertising) {
        BLEDevice::startAdvertising();
        isAdvertising = true;
    }
}

void BLEApp::setSleeping() {
    if (isAdvertising) {
        BLEDevice::stopAdvertising();
        isAdvertising = false;
    }
}

void BLEApp::setup()
{
    BLEDevice::init("Pepe TWatch S3");
    BLEServer * pServer = BLEDevice::createServer();
    BLEService * pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );

    pCharacteristic->setCallbacks(new MyCallbacks(Serial));
    pCharacteristic->setValue("Hello, World!");
    pService->start();

    BLEAdvertising * pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x06);
    BLEDevice::startAdvertising();
    isAdvertising = true;

    Serial.println("Waiting for a client connection...");
}

BLEApp::BLEApp() {
    setup();
}