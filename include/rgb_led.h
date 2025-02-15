#pragma once

#include <Arduino.h>
#include "main.h"

/**
 * @brief RGB led headers
 *
 */
#define RMT_DATA(D0, L0, D1, L1)                                          \
    ((uint32_t)D0 | (((uint32_t)L0) << 15UL) | (((uint32_t)D1) << 16UL) | \
     (((uint32_t)L1) << 31UL))

#define RGB24BIT(red, green, blue) ((uint32_t)blue | ((uint32_t)red << 8) | ((uint32_t)green << 16))

esp_err_t rgb_set_period(const float f, const TickType_t xTicksToWait);
float rgb_get_period();
bool rgb_get_state();

esp_err_t rgb_init_task(UBaseType_t uxPriority);
esp_err_t rgb_stop_task();