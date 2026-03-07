#pragma once
#include <stdint.h>
#include <Arduino.h>
#include "logger.h"


// # --- MODELS --- 
struct GreenhouseCmdModel_t {
    char modeCtx;
    int16_t rssi;
    int16_t samplingSensors; 
    int16_t temperatureMaxi;    // 1/10 degréé 204 ==> 20.4
    int16_t temperatureMini;
    int16_t hygrometryMaxi;    // 1/10 degréé 204 ==> 20.4
    int16_t hygrometryMini;
    int16_t ambientLightMaxi; 
    int16_t ambientLightMini;

    uint8_t digital_force_enable;
    uint8_t digital_force_value ;
    uint8_t pwm_force_enable ; 
    int    pwm_force_value[8]; 
    bool   modelDirty;
};
extern GreenhouseCmdModel_t cmdModel;
String  cmdModelToJson(const GreenhouseCmdModel_t* model) ;

// # --- MODELS --- 
struct GreenhouseSensorsModel_t {
    int16_t temperature;    // 1/10 degréé 204 ==> 20.4
    int16_t humidity;
    int16_t light ;
    bool   modelDirty;
};
extern GreenhouseSensorsModel_t sensorsModel;
String  sensorsModelToJson(const GreenhouseSensorsModel_t* model) ;





