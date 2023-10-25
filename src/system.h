#pragma once
#ifndef HOMESPANGARAGEDOOROPENER_SYSTEM_H
#define HOMESPANGARAGEDOOROPENER_SYSTEM_H
#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

//nvs_handle mqtt_config_nvs_handle;
#endif //HOMESPANGARAGEDOOROPENER_SYSTEM_H
