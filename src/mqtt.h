#pragma once

#ifndef HOMESPANGARAGEDOOROPENER_MQTT_H
#define HOMESPANGARAGEDOOROPENER_MQTT_H
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "system.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>

TaskHandle_t mqtt_task_handle;

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

volatile bool door_open = false;

char host_ip[16];
char device_name[50];
char user[50];
char password[50];
uint16_t port;

void subscribe() {
    // TODO: subscribe to topics that report door state
}

void callback(char* topic, byte* payload, unsigned int length) {
    // TODO: this is where the variables of door state are set, then in homespan code, these variable are polled to check door status
}

void reconnect() {
    while (!mqtt_client.connected()) {
        if (mqtt_client.connect(device_name, user, password)) {
            subscribe();
        }
        else vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    Serial.println("MQTT Connected");
}


[[noreturn]] void mqtt_task(void*) {
    while (true) {
        mqtt_client.loop();
        if (WiFi.status() == WL_CONNECTED && !mqtt_client.connected()) {
            reconnect();
        } else if (WiFi.status() != WL_CONNECTED) {
            ESP.restart();
        }
    }

}

//char* ask_for_user_input(const char* question) {
//    Serial.println(question);
//    while (!Serial.available()) {
//
//    }
//
//    String answer;
//
//    if (Serial.available()) {
//        answer = Serial.readStringUntil('\n');
//    }
//
//    char* retVal = static_cast<char *>(malloc((answer.length() + 1) * sizeof(char)));
//    strcpy(retVal, answer.c_str());
//
//    return retVal;
//
//}
//

void setupMqtt() {

    while (WiFi.status() != WL_CONNECTED) { // wait for wifi to connect first
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

//    nvs_open("SAVED_DATA",NVS_READWRITE,&mqtt_config_nvs_handle);
//
//    size_t required_size;

//    if (!nvs_get_str(mqtt_config_nvs_handle, "host_ip", NULL, &required_size)) { // found key
//        host_ip = static_cast<char *>(malloc(required_size));
//        nvs_get_str(mqtt_config_nvs_handle, "host_ip", host_ip, &required_size);
//    } else {
//        host_ip = ask_for_user_input("what is the host ip");
//        nvs_set_str(mqtt_config_nvs_handle, "host_ip", host_ip);
//    }

//    if (nvs_get_u16(mqtt_config_nvs_handle, "port", &port) == ESP_ERR_NVS_NOT_FOUND) {
//        port = atoi(ask_for_user_input("what port"));
//        nvs_set_u16(mqtt_config_nvs_handle, "port", port);
//    }

//    if (!nvs_get_str(mqtt_config_nvs_handle, "user", NULL, &required_size)) { // found key
//        user = static_cast<char *>(malloc(required_size));
//        nvs_get_str(mqtt_config_nvs_handle, "user", user, &required_size);
//    } else {
//        user = ask_for_user_input("what is the mqtt username");
//        nvs_set_str(mqtt_config_nvs_handle, "user", user);
//    }
//
//    if (!nvs_get_str(mqtt_config_nvs_handle, "password", NULL, &required_size)) { // found key
//        password = static_cast<char *>(malloc(required_size));
//        nvs_get_str(mqtt_config_nvs_handle, "password", password, &required_size);
//    } else {
//        password = ask_for_user_input("what is the password for that user");
//        nvs_set_str(mqtt_config_nvs_handle, "password", password);
//    }

    StaticJsonDocument<2048> config;

    if (SPIFFS.exists("/mqtt.json")) {
        File config_file = SPIFFS.open("/mqtt.json", FILE_READ);
        if (config_file) {
            DeserializationError error = deserializeJson(config, config_file);
            if (!error) {
                config_file.close();
                port = config["port"].as<uint16_t>();
                strcpy(host_ip, config["host_ip"]);
                strcpy(device_name, config["device_name"]);
                strcpy(user, config["user"]);
                strcpy(password, config["password"]);

                Serial.println(host_ip);
                Serial.println(port);
                Serial.println(user);
                Serial.println(password);

                mqtt_client.setBufferSize(2048);
                mqtt_client.setServer(host_ip, port);

                reconnect(); // it will crash if wifi is not connected
                mqtt_client.setCallback(callback);

                xTaskCreatePinnedToCore(
                        &mqtt_task,
                        "MQTT Task",
                        10240,
                        NULL,
                        1,
                        &mqtt_task_handle,
                        app_cpu
                );

                return;
            }
        }
    }

    Serial.print("Setup MQTT Failed");


//    nvs_close(mqtt_config_nvs_handle);


}


#endif //HOMESPANGARAGEDOOROPENER_MQTT_H
