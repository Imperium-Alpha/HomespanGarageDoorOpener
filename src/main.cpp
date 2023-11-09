#include <Arduino.h>
#include "garageDoorOpener.h"
#include "mqtt.h"
#include "system.h"
#include <SPIFFS.h>

GarageDoorOpenerAccessory* device_handle;
SpanUserCommand* erase_mqtt_config_cmd_handle;


void setup() {
    Serial.begin(115200);

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

    Serial.println("setting up...");
    homeSpan.autoPoll();
}

void loop() {
}