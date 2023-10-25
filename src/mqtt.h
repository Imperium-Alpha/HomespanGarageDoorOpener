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

volatile uint8_t door_state = -1; // 0 = open, 1 = closed, -1 = unknown, -2 = invalid state

char host_ip[16];
char device_name[50];
char user[50];
char password[50];
uint16_t port;

StaticJsonDocument<2048> payload_buffer;

void subscribe() {
    mqtt_client.subscribe("home/stat/garage_door_state");
}

void callback(char* topic, byte* payload, unsigned int length) {
    payload_buffer.clear();
    DeserializationError error = deserializeJson(payload_buffer, payload, length);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
    }

    if (strcmp(topic, "home/stat/garage_door_state") == 0) {
        if (payload_buffer.containsKey("door_state")) {
            if (strcmp(payload_buffer["door_state"], "open") == 0) {
                door_state = 0;
            } else if (strcmp(payload_buffer["door_state"], "closed") == 0) {
                door_state = 1;
            } else if (strcmp(payload_buffer["door_state"], "unknown") == 0) {
                door_state = -1;
            } else {
                door_state = -2;
            }
        }
    }
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


void setupMqtt() {

    while (WiFi.status() != WL_CONNECTED) { // wait for wifi to connect first
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }


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

}


#endif //HOMESPANGARAGEDOOROPENER_MQTT_H
