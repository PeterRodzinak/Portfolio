#include <lvgl.h>
#include <LilyGoLib.h>
#include <LV_Helper.h>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <string>
#include <WebServer.h>
#include <cstring>
#include <filesystem>
#include <spiffs_config.h>
#include "libs/Arduino_JSON/src/Arduino_JSON.h"
#include "FS.h"
#include "FFat.h"
#include <WiFiClientSecure.h>

#include "libs/arduino_base64.hpp"
#include "utils.hpp"
#include "Widgets/WidgetApp.hpp"
#include "Widgets/Clock.hpp"
#include "Widgets/Forecast.hpp"
#include "Widgets/Stopwatch.hpp"
#include "Widgets/WifiApp.hpp"
#include "Widgets/Pedometer.hpp"
#include "Widgets/BLEApp.hpp"
#include "Widgets/SmartHomeRemote.hpp"
#include "Utilities/HTTPSClient.hpp"

lv_style_t backgroundStyle;

enum appEnum {SmartHome,
              Pedometer,
              Clock,
              Downloader,
              Forecast,
              Wifi,
              Bluetooth,
              Terminal,
              Stopwatch,
              Testing};

lv_obj_t * mainTileview;
std::unordered_map<lv_obj_t *, size_t> tilePosMap;
size_t curMainTileIndex;
std::vector<std::shared_ptr<WidgetApp>> apps;

lv_obj_t * ipLabel;
lv_obj_t * downloaderLabel;

bool pmu_flag = false;
bool sensorIRQ = false;

WebServer server(80);
fs::File uploadFile;

void setPMUFlag() {
    pmu_flag = true;
}

void setSensorFlag() {
    sensorIRQ = true;
}

void createMainApps();

void createDownloaderUI();

void createUI();

void partloop(esp_partition_type_t part_type) {
    esp_partition_iterator_t iterator = NULL;
    const esp_partition_t *next_partition = NULL;
    iterator = esp_partition_find(part_type, ESP_PARTITION_SUBTYPE_ANY, NULL);
    while (iterator) {
        next_partition = esp_partition_get(iterator);
        if (next_partition != NULL) {
            Serial.printf("partition addr: 0x%06x; size: 0x%06x; label: %s\n", next_partition->address, next_partition->size, next_partition->label);  
        iterator = esp_partition_next(iterator);
    }
  }
}
 
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    
    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.printf("  FILE: %20s\tSIZE: %u\n\r", file.name(), file.size());
        }
	file.close();
        file = root.openNextFile();
    }
}

bool setupGlobalClient() {
    File certFile = FFat.open("/certificates/x509_crt_bundle.bin", "r");
    if (!certFile) return false;

    uint8_t * caCert;
    while (certFile.available()) {
        caCert = (uint8_t *)calloc(certFile.size(), sizeof(uint8_t));
        certFile.readBytes((char *)caCert, certFile.size());
    }
    certFile.close();

    // Load cert into client
    GlobalClient.setCACertBundle(caCert);
    return true;
}

void FFatInit() {
    Serial.setDebugOutput(true);

    Serial.println("Partition list:");
    partloop(ESP_PARTITION_TYPE_APP);
    partloop(ESP_PARTITION_TYPE_DATA);

    Serial.println("\n\nTrying to mount ffat partition if present");
 
    // Only allow one file to be open at a time instead of 10, saving 9x4 - 36KB of RAM
    if(!FFat.begin( 0, "", 5 )){
        Serial.println("FFat Mount Failed");
        return;
    }
 
    Serial.println("File system mounted");
    Serial.printf("Total space: %10lu\n", FFat.totalBytes());
    Serial.printf("Free space:  %10lu\n\n", FFat.freeBytes());
    listDir(FFat, "/", 5);
    Serial.setDebugOutput(false);
}

void lowPowerEnergyHandler() {
    Serial.println("Enter light sleep mode!");
    watch.decrementBrightness(0);

    watch.clearPMU();

    watch.configreFeatureInterrupt(
        SensorBMA423::INT_STEP_CNTR |   // Pedometer interrupt
        SensorBMA423::INT_ACTIVITY |    // Activity interruption
        SensorBMA423::INT_TILT |        // Tilt interrupt
        // SensorBMA423::INT_WAKEUP |      // DoubleTap interrupt
        SensorBMA423::INT_ANY_NO_MOTION,// Any  motion / no motion interrupt
        false);

    sensorIRQ = false;
    pmu_flag = false;

    for (auto & app : apps) {
        app->setSleeping();
    }

    setCpuFrequencyMhz(10);
    while (!pmu_flag && !sensorIRQ && !watch.getTouched()) {
        delay(300);
    }

    setCpuFrequencyMhz(240);

    for (size_t i = 0; i < apps.size(); i++) {
        if (i == curMainTileIndex)
            apps[i]->setActive();
        else
            apps[i]->setBackground();
    }

    watch.configreFeatureInterrupt(
        SensorBMA423::INT_STEP_CNTR |   // Pedometer interrupt
        SensorBMA423::INT_ACTIVITY |    // Activity interruption
        SensorBMA423::INT_TILT |        // Tilt interrupt
        // SensorBMA423::INT_WAKEUP |      // DoubleTap interrupt
        SensorBMA423::INT_ANY_NO_MOTION,// Any  motion / no motion interrupt
        true);

    lv_disp_trig_activity(NULL);
    lv_task_handler();



    watch.incrementalBrightness(100);
}

void setup() {
    Serial.begin(115200);

    watch.begin();

    watch.enableButtonBatteryCharge();

    beginLvglHelper();

    FFatInit();

    delay(1000);
    if (setupGlobalClient()) {
        Serial.println("Client could not load the certificates.");
    }

    watch.configAccelerometer();

    watch.enableAccelerometer();

    watch.enablePedometer();

    watch.configInterrupt();

    watch.enableFeature(SensorBMA423::FEATURE_STEP_CNTR |
                        SensorBMA423::FEATURE_ANY_MOTION |
                        SensorBMA423::FEATURE_NO_MOTION |
                        SensorBMA423::FEATURE_ACTIVITY |
                        SensorBMA423::FEATURE_TILT |
                        SensorBMA423::FEATURE_WAKEUP,
                        true);

    watch.enablePedometerIRQ();
    watch.enableTiltIRQ();
    watch.enableWakeupIRQ();
    watch.enableAnyNoMotionIRQ();
    watch.enableActivityIRQ();

    createMainApps();

    createUI();

    for (auto & app : apps) {
        app->setBackground();
    }

    apps[0]->setActive();

    curMainTileIndex = 0;

    watch.attachPMU(setPMUFlag);

    watch.attachBMA(setSensorFlag);
}

static void tile_changed_event_cb(lv_event_t * e) {
    lv_obj_t * tile = lv_tileview_get_tile_act(lv_event_get_target(e));

    if (tilePosMap[tile] > 0) {
        apps[tilePosMap[tile] - 1]->setBackground();
    }

    if (tilePosMap[tile] < apps.size() - 1) {
        apps[tilePosMap[tile] + 1]->setBackground();
    }
    
    apps[tilePosMap[tile]]->setActive();
    curMainTileIndex = tilePosMap[tile];
    //Serial.printf("Current tile index: %u\n", curMainTileIndex);
}

void createMainApps() {
    lv_style_init(&backgroundStyle);
    lv_style_set_bg_color(&backgroundStyle, lv_color_black());

    mainTileview = lv_tileview_create(lv_scr_act());
    lv_obj_add_style(mainTileview, &backgroundStyle, LV_PART_MAIN);
    lv_obj_set_size(mainTileview, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_add_event_cb(mainTileview, tile_changed_event_cb, LV_EVENT_SCROLL_END, NULL);

    apps = std::vector<std::shared_ptr<WidgetApp>>();
    apps.emplace_back(clockApp::getApp());
    apps.emplace_back(StopwatchApp::getApp());
    apps.emplace_back(forecastApp::getApp(&GlobalClient));
    apps.emplace_back(pedometerApp::getApp());
    apps.emplace_back(smartHomeApp::getApp());
    apps.emplace_back(wifiApp::getApp());
}

void createUI() {
    for (size_t i = 0; i < apps.size(); i++) {
        auto tile = lv_tileview_add_tile(
            mainTileview,
            0,
            i,
            (i < apps.size() - 1 ? LV_DIR_BOTTOM : 0) | (i == 0 ? 0 : LV_DIR_TOP)
        );
        tilePosMap[tile] = i;
        apps[i]->createUI(tile);
    }
}

void loop()
{
    if (pmu_flag) {
        watch.readPMU();
        if (watch.isPekeyShortPressIrq()) {
            watch.setSleepMode(PMU_BTN_WAKEUP);
            watch.clearPMU();
            watch.sleep();
        }
        watch.clearPMU();
    }
    if (sensorIRQ) {
        sensorIRQ = false;
        // The interrupt status must be read after an interrupt is detected
        uint16_t status = watch.readBMA();

        if (watch.isPedometer()) {
            pedometerApp::getApp()->updateCounter();
        }
    }

    bool screenTimeout = lv_disp_get_inactive_time(NULL) > 15000;
    if (screenTimeout) {
        lowPowerEnergyHandler();
    }
    
    lv_task_handler();

    delay(5);
}
