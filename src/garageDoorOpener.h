#pragma once

#ifndef HOMESPANGARAGEDOOROPENER_GARAGEDOOROPENER_H
#define HOMESPANGARAGEDOOROPENER_GARAGEDOOROPENER_H

#include <HomeSpan.h>
#include "relay.h"
#include "mqtt.h"
#define CHECK_INTERVAL 8000  // this must be greater than the time taken for door to completely open or close

struct AccessoryInformation: Service::AccessoryInformation {
    SpanCharacteristic* name;
    SpanCharacteristic* identity;

    explicit AccessoryInformation(const char* accessory_name): Service::AccessoryInformation() {
        name = new Characteristic::Name(accessory_name);
        identity = new Characteristic::Identify();
    }
};

struct GarageDoorOpener: Service::GarageDoorOpener {

    SpanCharacteristic* current;
    SpanCharacteristic* target;
    SpanCharacteristic* obstruction;

    GarageDoorOpener(): Service::GarageDoorOpener() {
        current = new Characteristic::CurrentDoorState(1); // default closed
        target = new Characteristic::TargetDoorState(1);
        obstruction = new Characteristic::ObstructionDetected(false);

        Serial.println("Setting up Garage Door Opener");
    }

    boolean update() override {
        if (target->getNewVal() == 0 && current->getVal() == 1) {
            switch_on_relay(200);
            current->setVal(2);
            Serial.println("Home app is opening the garage door");

        } else if (target->getNewVal() == 1 && current->getVal() == 0) {
            switch_on_relay(200);
            current->setVal(3);
            Serial.println("Home app is closing the garage door");
        } else if (current->getVal() == 2 || current->getVal() == 3) {
            // e-stop during door closing or opening
            switch_on_relay(200);
            if (current->getVal() == 2 && target->getNewVal() == 1) {
                // e stop is trigger when door is opening, (target state changed from open 0 to closed 1)
                target->setVal(0); // set target to open, so user can now close to door.
            } else if (current->getVal() == 3 && target->getNewVal() == 0) {
                target->setVal(1);
            }
            current->setVal(4); // door is stopped
        } else if (current->getVal() == 4 && target->getNewVal() == 0) {
            switch_on_relay(200);
            current->setVal(2);
        } else if (current->getVal() == 4 && target->getNewVal() == 1) {
            switch_on_relay(200);
            current->setVal(3);
        }

        return true;
    }

    void loop() override {
        if (current->timeVal() > CHECK_INTERVAL) {
            if (door_state == 1) {
                current->setVal(1);
                target->setVal(1);
                obstruction->setVal(false);
            } else if (door_state == 0) {
                current->setVal(0);
                target->setVal(0);
                obstruction->setVal(false);
            } else {
                if (current->getVal() == 2 && target->getVal() == 0) {
                    Serial.println("door obstructed when opening");
                    obstruction->setVal(true);
                    current->setVal(4);
                } else if (current->getVal() == 3 && target->getVal() == 1) {
                    Serial.println("door obstructed when closing");
                    obstruction->setVal(true);
                    current->setVal(4);
                } else {
                    current->setVal(current->getVal());
                }
            }
        }
    }
};

struct GarageDoorOpenerAccessory: SpanAccessory {
    SpanService* garageDoorOpener;
    SpanService* accessoryInformation;

    GarageDoorOpenerAccessory(): SpanAccessory() {
        accessoryInformation = new AccessoryInformation("Garage Door Opener");
        garageDoorOpener = new GarageDoorOpener();

    }
};

#endif //HOMESPANGARAGEDOOROPENER_GARAGEDOOROPENER_H
