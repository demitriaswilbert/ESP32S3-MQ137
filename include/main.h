#pragma once

#include <Arduino.h>
#include <MQUnifiedsensor.h>

typedef struct {
    float ppm;
    float r0;
    float rs;
    float VRL;
} sensor_param_t;

typedef struct {
    sensor_param_t mq135;
    sensor_param_t mq137;
} sensor_data_t;

typedef enum {
    MQ_CAL_OK,
    MQ_CAL_ERR_OPEN_CIRCUIT,
    MQ_CAL_ERR_GROUND
} mq_cal_e;

static inline mq_cal_e calibrate(MQUnifiedsensor& sensor, float ratio_clean_air)
{
    // Serial.print("Calibrating please wait.");
    float calcR0 = 0;
    for (int i = 1; i <= 10; i++)
    {
        sensor.update(); // Update data, the arduino will read the voltage from the analog pin
        calcR0 += sensor.calibrate(ratio_clean_air);
    }
    sensor.setR0(calcR0 / 10);

    if (isinf(calcR0))
        return MQ_CAL_ERR_OPEN_CIRCUIT;
    if (calcR0 == 0)
        return MQ_CAL_ERR_GROUND;
    return MQ_CAL_OK;
}


void recalibrate_mq135();
void recalibrate_mq137();

extern SemaphoreHandle_t sensor_data_lock;
extern sensor_data_t sensor_data;