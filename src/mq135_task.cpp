// Include the library
#include <MQUnifiedsensor.h>
#include "main.h"
// Definitions
#define Voltage_Resolution 5
#define pin 7                 
#define ADC_Bit_Resolution 12  // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6 // RS / R0 = 3.6 ppm
// #define calibration_button 13 //Pin to calibrate your sensor

static void update_data_to_main(sensor_param_t& sensor_param)
{
    if (xSemaphoreTake(sensor_data_lock, portMAX_DELAY) == pdTRUE) {
        sensor_data.mq135 = sensor_param;
        xSemaphoreGive(sensor_data_lock);
    }
}

static bool is_calibrated = false;

void recalibrate_mq135() {
    is_calibrated = false;
}

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

    /*****************************  MQ CAlibration ********************************************/
    // MQ135.serialDebug(true);
    sensor_param_t sensor_param = {NAN, NAN, NAN};

    while (calibrate(MQ135, RatioMQ135CleanAir) != MQ_CAL_OK) 
    {    
        update_data_to_main(sensor_param);
        vTaskDelay(1000);
    }

    while (true)
    {
        if (!is_calibrated)
        {
            sensor_param = {NAN, NAN, NAN};
            update_data_to_main(sensor_param);
            while (calibrate(MQ135, RatioMQ135CleanAir) != MQ_CAL_OK) 
            {    
                update_data_to_main(sensor_param);
                vTaskDelay(1000);
            }
            is_calibrated = true;
        }

        MQ135.update();
        sensor_param.ppm = MQ135.readSensor();
        sensor_param.VRL = MQ135.getVoltage(false);
        sensor_param.r0 = MQ135.getR0();
        sensor_param.rs = MQ135.getRS();
        update_data_to_main(sensor_param);
        delay(200);          // Sampling frequency
    }
}