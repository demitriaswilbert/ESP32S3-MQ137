#include <Arduino.h>
#include <MQ137.h>

#define MQ137_ANALOG_PIN 6

// Replace with the value obtained when calibrating the sensor
#define MQ137_R0 26.21f

MQ137 mq137(MQ137_ANALOG_PIN, MQ137_R0, true);

void setup()
{
    Serial.begin(115200);
    delay(5000);
    mq137.begin();
    mq137.getRo();
}

void loop()
{
    float ppm = mq137.getPPM();
    float vrl = mq137.getVRL();
    Serial.printf("PPM: %f, VRL: %f\n", ppm, vrl);
    delay(1000);
}