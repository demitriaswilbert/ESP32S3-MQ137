#pragma once

#include <Arduino.h>

typedef struct {
    float mq135_ppm;
    float mq137_ppm;
} sensor_data_t;

extern SemaphoreHandle_t sensor_data_lock;
extern sensor_data_t sensor_data;