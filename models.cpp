#include "src\models.h"
#include <ArduinoJson.h>

GreenhouseCmdModel_t cmdModel = {
    .modeCtx = 'S',
    .rssi = -120 , 
    .samplingSensors = 10 ,
    .temperatureMaxi = 250,
    .temperatureMini = 120,
    .hygrometryMaxi  = 85,     
    .hygrometryMini  = 40, 
    .ambientLightMaxi = 1500, 
    .ambientLightMini = 150 ,
    .digital_force_enable  = 0b00000000,  //  0 => Mode force  2-7 ==> gpio(bit)
    .digital_force_value   = 0b00000000,
    .pwm_force_enable = 0,
    .pwm_force_value = {0,0,0,0,0,0,0,0} , 
    .modelDirty = true
};

GreenhouseSensorsModel_t sensorsModel  {
    .temperature = 888,  // 1/10 degréé 204 ==> 20.4
    .humidity  = 88,
    .light  = 888,
    .modelDirty = true 
};


String cmdModelToJson(const GreenhouseCmdModel_t* model)
{
    StaticJsonDocument<750> doc;
    doc["mode"] = (String)model->modeCtx;

    doc["rssi"]    =  model->rssi;

    doc["samplingSensors"]    =  model->samplingSensors;
    
    doc["temperature"]["max"] = model->temperatureMaxi * 0.1f;
    doc["temperature"]["min"] = model->temperatureMini * 0.1f;

    doc["hygrometry"]["max"] = model->hygrometryMaxi;
    doc["hygrometry"]["min"] = model->hygrometryMini;

    doc["ambientLight"]["max"] = model->ambientLightMaxi;
    doc["ambientLight"]["min"] = model->ambientLightMini;

    doc["digital"]["forceEnable"] = model->digital_force_enable;
    doc["digital"]["forceValue"] = model->digital_force_value;

    doc["pwm"]["forceEnable"] = model->pwm_force_enable;

    JsonArray pwmArray = doc["pwm"]["forceValue"].to<JsonArray>();
    for (int i = 0; i < 8; i++)
        pwmArray.add(model->pwm_force_value[i]);

    String output;
    serializeJson(doc, output);

    return output;
}

String sensorsModelToJson(const GreenhouseSensorsModel_t* model)
{
    StaticJsonDocument<512> doc;

    doc["temperature"]  = model->temperature * 0.1f;
    doc["humidity"]     = model->humidity;
    doc["light"]        = model->light;
    
    String output;
    serializeJson(doc, output);
    return output;
}


