#include <Arduino.h>
#include <Arduino_JSON.h>
#include "main.h"
#include "rgb_led.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>

extern void mq135_task(void* param);
extern void mq137_task(void* param);

SemaphoreHandle_t sensor_data_lock = NULL;
sensor_data_t sensor_data = {0};

float dht_humid = 0, dht_temp = 0;

void dht22_task(void* param)
{
    DHT dht(4, DHT22);
    dht.begin();
    while (true)
    {
        dht.read(true);
        dht_humid = dht.readHumidity(false);
        dht_temp = dht.readTemperature(false);
        vTaskDelay(2000);
    }
    vTaskDelete(NULL);
}

void sensor_read_task(void* param)
{
    UBaseType_t task_prio = uxTaskPriorityGet(NULL);
    while (true)
    {
        if (Serial.available() > 0x1000)
        {
            if (task_prio == 0) {
                vTaskPrioritySet(NULL, 3);
                task_prio = uxTaskPriorityGet(NULL);
            }
        }
        else if (task_prio > 0) {
            vTaskPrioritySet(NULL, 0);
            task_prio = uxTaskPriorityGet(NULL);
        }
        int c = Serial.read();
        if (c != -1) process_char(Serial, c);
        else vTaskDelay(1);
    }
}

void sensor_log_task(void* param)
{
    sensor_param_t mq135_param, mq137_param;
    char buf[1000];
    while (true)
    {
        if (xSemaphoreTake(sensor_data_lock, portMAX_DELAY) == pdTRUE)
        {
            mq135_param = sensor_data.mq135;
            mq137_param = sensor_data.mq137;
    
            xSemaphoreGive(sensor_data_lock);
            int len = sprintf(buf,    "----------------------------------\n");
            len += sprintf(buf + len, "Temp   | %10.5f | %10.5f |\n", dht_temp, dht_humid);
            len += sprintf(buf + len, "Sensor | MQ135      | MQ137      |\n");
            len += sprintf(buf + len, "----------------------------------\n");
            len += sprintf(buf + len, "PPM    | %10.5f | %10.5f |\n", mq135_param.ppm, mq137_param.ppm);
            len += sprintf(buf + len, "VRL    | %10.5f | %10.5f |\n", mq135_param.VRL, mq137_param.VRL);
            len += sprintf(buf + len, "RS     | %10.5f | %10.5f |\n", mq135_param.rs, mq137_param.rs);
            len += sprintf(buf + len, "R0     | %10.5f | %10.5f |\n", mq135_param.r0, mq137_param.r0);
            len += sprintf(buf + len, "----------------------------------\n\n");
            Serial.write(buf, len);
        }
    
        if (!digitalRead(0)) {
            recalibrate_mq135();
            recalibrate_mq137();
            vTaskDelay(250);
        }
        vTaskDelay(250);
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.setRxBufferSize(0x2000);
    
    sensor_data_lock = xSemaphoreCreateMutex();
    pinMode(0, INPUT_PULLUP);

    // xTaskCreate(mq135_task, "mq135", 4096, NULL, 1, NULL);
    xTaskCreate(mq137_task, "mq137", 4096, NULL, 1, NULL);
    xTaskCreate(sensor_read_task, "sensor_read_task", 4096, NULL, 0, NULL);
    xTaskCreate(sensor_log_task, "sensor_log_task", 4096, NULL, 1, NULL);
    xTaskCreate(dht22_task, "dht22_task", 4096, NULL, 2, NULL);

    vTaskDelete(NULL);
}

void loop()
{
    
}