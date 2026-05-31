#include "BT.hpp"

void MyCallbacks::onWrite(BLECharacteristic * pCharacreristic) {
    std::string value = pCharacreristic->getValue();
    if (value.length() > 0) {
        serial.println("Received value:");
        serial.printf("%s\n\r", value.c_str());
    }
}