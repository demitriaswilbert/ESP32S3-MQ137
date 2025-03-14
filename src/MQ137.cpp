#include "MQ137.h"

MQ137::MQ137(uint8_t mq137_pin, float _r0, bool _mq137_debug)
{
    MQ137_PIN = mq137_pin;
    mq137_debug = _mq137_debug;
    MQ137_R0 = _r0;
}

void MQ137::begin(){
    if(mq137_debug){
        MQ137::mq137DebugPrint();
    }
}

float MQ137::get_MQ137_R0(){
    return MQ137_R0;
}

/**
 * @brief Obtener la resistencia del sensor a definir en MQ137_R0
 * 
 * @param analog_pin Pin analogico usado por el sensor MQ137
 * @return float 
 */
float MQ137::getRo(){
    float analog_value = 0;
    float Rs;
    float Ro;
    for (uint16_t i = 0; i < MQ137_CICLE_TEST; i++)
        analog_value += analogRead(MQ137_PIN);

    analog_value /= MQ137_CICLE_TEST;
    Serial.print("Average analog value\t");
    Serial.println(analog_value);
    
    MQ137_VRL = getVoltage();
    // VRL= resultado analogico a voltaje

    Serial.print("Average voltage\t");
    Serial.println(MQ137_VRL);
    Rs = ((MQ137_VCC_BOARD_IN / MQ137_VRL) - 1) * MQ137_RL;
    // Rs = the resistance value of the sensor

    Serial.print("Resistencia del sensor\t");
    Serial.println(Rs);
    Ro = Rs / AIR_CLEAR;
    // Ro = resistance value in clean air
    return Ro;
}

/**
 * @brief Get PPM from sensor 
 * 
 * @return float 
 */
float MQ137::getPPM(){
    float Rs;
    float ratio;
    float ppm;
    MQ137_VRL = getVoltage();
    Rs = ((MQ137_VCC_BOARD_IN * MQ137_RL) / MQ137_VRL) - MQ137_RL;
    ratio = Rs / MQ137_R0;
    ppm = pow(10, ((log10(ratio) - MQ137_MIDPOINT_NH3) / MQ137_SLOPE_NH3));
    return ppm;
}

void MQ137::mq137DebugPrint(){
    Serial.println("--------------------------------------------------");
    Serial.println("Variables afectadas a sensor MQ137");
    Serial.print("R0: ");
    Serial.println(MQ137_R0);
    Serial.print("AIR_CLEAR: ");
    Serial.println(AIR_CLEAR);
    Serial.print("RL: ");
    Serial.println(MQ137_RL);
    Serial.print("CICLE_TEST: ");
    Serial.println(MQ137_CICLE_TEST);
    Serial.print("VCC_BOARD_IN: ");
    Serial.println(MQ137_VCC_BOARD_IN);
    Serial.print("MAX_ADC_VALUE: ");
    Serial.println(MQ137_MAX_ADC_VALUE);
    Serial.print("SLOPE_NH3: ");
    Serial.println(MQ137_SLOPE_NH3);
    Serial.print("MIDPOINT_NH3: ");
    Serial.println(MQ137_MIDPOINT_NH3);
    Serial.println("--------------------------------------------------");
}