#include <Arduino.h>
#include <Arduino_JSON.h>
// #include <MQ137.h>
#include "main.h"

// #define MQ137_ANALOG_PIN 6

// Replace with the value obtained when calibrating the sensor
// #define MQ137_R0 26.21f

// MQ137 mq137(MQ137_ANALOG_PIN, MQ137_R0, true);

extern void mq135_task(void* param);
extern void mq137_task(void* param);

SemaphoreHandle_t sensor_data_lock = NULL;
sensor_data_t sensor_data = {0};

void setup()
{
    Serial.begin(115200);
    
    sensor_data_lock = xSemaphoreCreateMutex();

    xTaskCreate(mq135_task, "mq135", 4096, NULL, 1, NULL);
    xTaskCreate(mq137_task, "mq137", 4096, NULL, 1, NULL);
    pinMode(0, INPUT_PULLUP);
}

void loop()
{
    static sensor_param_t mq135_param, mq137_param;
    if (xSemaphoreTake(sensor_data_lock, portMAX_DELAY) == pdTRUE)
    {
        mq135_param = sensor_data.mq135;
        mq137_param = sensor_data.mq137;

        xSemaphoreGive(sensor_data_lock);
        Serial.printf("----------------------------------\n");
        Serial.printf("Sensor | MQ135      | MQ137      |\n");
        Serial.printf("----------------------------------\n");
        Serial.printf("PPM    | %10.5f | %10.5f |\n", mq135_param.ppm, mq137_param.ppm);
        Serial.printf("VRL    | %10.5f | %10.5f |\n", mq135_param.VRL, mq137_param.VRL);
        Serial.printf("RS     | %10.5f | %10.5f |\n", mq135_param.rs, mq137_param.rs);
        Serial.printf("R0     | %10.5f | %10.5f |\n", mq135_param.r0, mq137_param.r0);
        Serial.printf("----------------------------------\n\n");

    }

    if (!digitalRead(0)) {
        recalibrate_mq135();
        recalibrate_mq137();
        vTaskDelay(250);
    }

    vTaskDelay(250);
}