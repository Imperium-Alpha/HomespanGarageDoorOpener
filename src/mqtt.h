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


void subscribe() {
    mqtt_client.subscribe("home/stat/garage_door_state");
}

void callback(char* topic, byte* payload, unsigned int length) {

    if (strcmp(topic, "home/stat/garage_door_state") == 0) {
        Serial.println("Getting MQTT Garage Door State Message");
        Serial.println(length);
        payload[length] = '\0';
        Serial.println((char*) payload);
        if (strcmp((char*) payload, "open") == 0) {
            Serial.println("Door state to open");
            door_state = 0;
        } else if (strcmp((char*) payload, "closed") == 0) {
            Serial.println("Door state to closed");
            door_state = 1;
        } else if (strcmp((char*) payload, "unknown") == 0) {
            Serial.println("Door state to unknown");
            door_state = -1;
        } else {
            Serial.println("Door state to wtf");
            door_state = -2;
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

        if (WiFi.status() == WL_CONNECTED && !mqtt_client.connected()) {
            reconnect();
        } else if (WiFi.status() != WL_CONNECTED) {
            ESP.restart();
        }
        mqtt_client.loop();
    }

}


void setupMqtt() {

    SPIFFS.begin();

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
                SPIFFS.end();

                return;
            }
        }
    }

    SPIFFS.end();

    Serial.print("Setup MQTT Failed");

}


#endif //HOMESPANGARAGEDOOROPENER_MQTT_H
