#include <Arduino.h>
#include "garageDoorOpener.h"
#include "mqtt.h"
#include "system.h"
#include <SPIFFS.h>

GarageDoorOpenerAccessory* device_handle;
SpanUserCommand* erase_mqtt_config_cmd_handle;


void setup() {
    Serial.begin(115200);
    SPIFFS.begin();
    initialize_relay();

    homeSpan.setLogLevel(2);
    homeSpan.setWifiCallback(setupMqtt);
    homeSpan.setPairingCode("24682468");
    homeSpan.setPairCallback([](bool paired) {
        if (paired) {
            Serial.println("User Message: Device Paired");
        } else {
            Serial.println("User Message: Device Unpaired");
        }
    });

    homeSpan.begin(Category::GarageDoorOpeners, "Garage Door Opener");
    device_handle = new GarageDoorOpenerAccessory();

//    erase_mqtt_config_cmd_handle = new SpanUserCommand('m', "Erase Mqtt Config", [] (const char* buf) {
//        nvs_open("SAVED_DATA",NVS_READWRITE,&mqtt_config_nvs_handle);
//
//        // TODO: erase all the key that store mqtt information in the nvs
//        nvs_erase_key(mqtt_config_nvs_handle, "host_ip");
////        nvs_erase_key(mqtt_config_nvs_handle, "port");
//        nvs_erase_key(mqtt_config_nvs_handle, "user");
//        nvs_erase_key(mqtt_config_nvs_handle, "password");
//        nvs_close(mqtt_config_nvs_handle);
//    });

    Serial.println("setting up...");
    homeSpan.autoPoll();
}

void loop() {
}