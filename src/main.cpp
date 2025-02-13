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
}

void loop()
{
    static float mq135_ppm, mq137_ppm;
    if (xSemaphoreTake(sensor_data_lock, portMAX_DELAY) == pdTRUE)
    {
        mq135_ppm = sensor_data.mq135_ppm;
        mq137_ppm = sensor_data.mq137_ppm;

        xSemaphoreGive(sensor_data_lock);
        Serial.printf("MQ135: %9.4f      MQ137: %9.4f\n", mq135_ppm, mq137_ppm);
    }
    vTaskDelay(200);
}