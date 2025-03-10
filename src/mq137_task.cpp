// Include the library
#include <MQUnifiedsensor.h>
#include "main.h"

// Definitions
#define Voltage_Resolution 3.3
#define pin 13                 // Analog input 0 of your arduino
#define ADC_Bit_Resolution 12  // For arduino UNO/MEGA/NANO
// #define RatioMQ137CleanAir 1 // RS / R0 = 3.6 ppm
#define RatioMQ137CleanAir 3.6 // RS / R0 = 3.6 ppm
// #define calibration_button 13 //Pin to calibrate your sensor

// #define MQ137_SLOPE_NH3 -0.243
#define MQ137_SLOPE_NH3 -0.2717285079732674
// #define MQ137_MIDPOINT_NH3 0.323
#define MQ137_MIDPOINT_NH3 -0.22184874961635637

static void update_data_to_main(sensor_param_t &sensor_param)
{
    if (xSemaphoreTake(sensor_data_lock, portMAX_DELAY) == pdTRUE)
    {
        sensor_data.mq137 = sensor_param;
        xSemaphoreGive(sensor_data_lock);
    }
}

static bool is_calibrated = false;

void recalibrate_mq137()
{
    is_calibrated = false;
}

/*
 * Program to measure gas in ppm using MQ sensor
 * Program by: B.Aswinth Raj
 * Website: www.circuitdigest.com
 * Dated: 28-12-2017
 */
#define tmp_RL 1 // The value of resistor RL is 47K
// #define tmp_m -0.263 //Enter calculated Slope
#define tmp_m -0.2616480 // Enter calculated Slope

// #define tmp_b 0.42 //Enter calculated intercept
#define tmp_b -0.22195929408 // Enter calculated intercept
#define tmp_Ro 33            // Enter found Ro value
#define tmp_MQ_sensor pin      // Sensor is connected to A4

void mq137_calculate(float* pVrl, float* pRs)
{
    float vrl = analogRead(tmp_MQ_sensor) * (3.3 / 4095.0);

    if (pVrl)
        *pVrl = vrl;
    if (pRs)
        *pRs = ((5.0 * tmp_RL) / vrl) - tmp_RL;
}

void mq137_calibrate(float *pR0)
{
    float sum_r0 = 0;
    for (int i = 0; i < 500; i++) {
        float f;
        mq137_calculate(NULL, &f);
        sum_r0 += f;
    }
    sum_r0 /= 500;
    *pR0 = sum_r0 / 3.6;
}

void mq137_task(void* param)
{
    sensor_param_t sensor_param = {NAN, NAN, NAN};
    float VRL;                                             // Voltage drop across the MQ sensor
    float Rs;                                              // Sensor resistance at gas concentration
    float ratio;    
    float measured_r0;
    // mq137_calibrate(&measured_r0);
    while (true)
    {
        if (!is_calibrated) 
        {
            sensor_param = {NAN, NAN, NAN};
            update_data_to_main(sensor_param);
            mq137_calibrate(&measured_r0);
            vTaskDelay(1000);
            is_calibrated = true;
        }
        mq137_calculate(&VRL, &Rs);
        sensor_param.VRL = VRL;
        sensor_param.rs = Rs;
        sensor_param.r0 = measured_r0;
        ratio = Rs / measured_r0;                                   // find ratio Rs/Ro
        float ppm = pow(10, ((log10(ratio) - tmp_b) / tmp_m)); // use formula to calculate ppm
        sensor_param.ppm = ppm;
        update_data_to_main(sensor_param);
        delay(200); // Sampling frequency
    }
}

void mq137_task1(void *param)
{
    MQUnifiedsensor MQ137("Arduino", Voltage_Resolution, ADC_Bit_Resolution, pin, "MQ-137");
    // Set math model to calculate the PPM concentration and the value of constants
    MQ137.setRegressionMethod(0); //_PPM =  a*ratio^b

    MQ137.setA(MQ137_SLOPE_NH3);
    MQ137.setB(MQ137_MIDPOINT_NH3); // Configure the equation to to calculate NH4 concentration

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
    MQ137.init();

    /*****************************  MQ CAlibration ********************************************/
    // MQ137.serialDebug(true);
    sensor_param_t sensor_param = {NAN, NAN, NAN};

    // while (calibrate(MQ137, RatioMQ137CleanAir) != MQ_CAL_OK)
    // {
    //     update_data_to_main(sensor_param);
    //     vTaskDelay(1000);
    // }

    while (true)
    {
        if (!is_calibrated)
        {
            sensor_param = {NAN, NAN, NAN};
            update_data_to_main(sensor_param);
            while (calibrate(MQ137, RatioMQ137CleanAir) != MQ_CAL_OK)
            {
                update_data_to_main(sensor_param);
                vTaskDelay(1000);
            }
            is_calibrated = true;
        }

        MQ137.update();
        sensor_param.ppm = MQ137.readSensor();
        sensor_param.VRL = MQ137.getVoltage(false);
        sensor_param.r0 = MQ137.getR0();
        sensor_param.rs = MQ137.getRS();
        update_data_to_main(sensor_param);
        delay(200); // Sampling frequency
    }
}