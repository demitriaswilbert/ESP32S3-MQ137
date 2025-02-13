// Include the library
#include <MQUnifiedsensor.h>
#include "main.h"
// Definitions
#define Voltage_Resolution 5
#define pin 7                 // Analog input 0 of your arduino
#define ADC_Bit_Resolution 12  // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6 // RS / R0 = 3.6 ppm
// #define calibration_button 13 //Pin to calibrate your sensor

// Declare Sensor

void mq135_task(void* param)
{
    MQUnifiedsensor MQ135("Arduino", Voltage_Resolution, ADC_Bit_Resolution, pin, "MQ-135");
    // Set math model to calculate the PPM concentration and the value of constants
    MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
    MQ135.setA(102.2);
    MQ135.setB(-2.473); // Configure the equation to to calculate NH4 concentration

    /*
      Exponential regression:
    GAS      | a      | b
    CO       | 605.18 | -3.937
    Alcohol  | 77.255 | -3.18
    CO2      | 110.47 | -2.862
    Toluen  | 44.947 | -3.445
    NH4      | 102.2  | -2.473
    Aceton  | 34.668 | -3.369
    */

    /*****************************  MQ Init ********************************************/
    // Remarks: Configure the pin of arduino as input.
    /************************************************************************************/
    MQ135.init();
    /*
      //If the RL value is different from 10K please assign your RL value with the following method:
      MQ135.setRL(10);
    */
    /*****************************  MQ CAlibration ********************************************/
    // Explanation:
    // In this routine the sensor will measure the resistance of the sensor supposedly before being pre-heated
    // and on clean air (Calibration conditions), setting up R0 value.
    // We recomend executing this routine only on setup in laboratory conditions.
    // This routine does not need to be executed on each restart, you can load your R0 value from eeprom.
    // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor

    while (true)
    {
        Serial.print("Calibrating please wait.");
        float calcR0 = 0;
        for (int i = 1; i <= 10; i++)
        {
            MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
            calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
            Serial.print(".");
        }
        MQ135.setR0(calcR0 / 10);
        Serial.println("  done!.");
    
        if (isinf(calcR0))
        {
            Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
            vTaskDelay(1000);
            continue;
        }
        if (calcR0 == 0)
        {
            Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
            vTaskDelay(1000);
            continue;
        }
        break;
    }
    
    /*****************************  MQ CAlibration ********************************************/
    // MQ135.serialDebug(true);

    while (true)
    {
        MQ135.update();      // Update data, the arduino will read the voltage from the analog pin
        // Serial.printf("MQ135 PPM: %f\n", MQ135.readSensor());
        float ppm = MQ135.readSensor();
        if (xSemaphoreTake(sensor_data_lock, portMAX_DELAY) == pdTRUE) {
            sensor_data.mq135_ppm = ppm;
            xSemaphoreGive(sensor_data_lock);
        }
        delay(200);          // Sampling frequency
    }
}