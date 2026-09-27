#include "src\sensors.h"

//extern TwoWire TwoWire_I2C1  ;
TwoWire TwoWire_I2C1(1);        // 👉 TON bus séparé

// --- Device list, definition , instance  --  
uint8_t BH1750_addr = 0x23 ;  
uint8_t SHT31_addr  = 0x44 ;  
uint8_t BME280_addr = 0x76 ;  


typedef struct { uint8_t addr; const char *label; bool isAvailable; } I2C_Device_List;
I2C_Device_List I2C_devices[] = {
    { BH1750_addr, "BH1750 Light Sensor" , false  },
    { SHT31_addr,  "SHT31  Plants Temp/Humidity" , false },
    { BME280_addr, "BME280 Exterior Temp/Humidity/Pressure" , false }

};
#define DEVICE_COUNT (sizeof(I2C_devices) / sizeof(I2C_devices[0]))

BH1750          lightSensor(BH1750_addr);
Adafruit_SHT31  plantAirSensor(&TwoWire_I2C1); //plantAirSensor
Adafruit_BME280 exteriorAirSensor ; // I2C

/*
    // status = bme.begin(0x76, &Wire2)
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
    }
    
    Serial.println("-- Default Test --");
    delayTime = 1000;

    Serial.println();
}
*/


int counter,counterBackup  = 0 ; 

// -- Functions 
void initSensors() {

  // --- I2C Hardware Sensors Check ---
  TwoWire_I2C1.begin(GPIO_I2C_1_SDA, GPIO_I2C_1_SCL);

  for (int i = 0; i < DEVICE_COUNT; i++) {
    uint8_t addr = I2C_devices[i].addr;
    const char *name = I2C_devices[i].label;
  
    bool rc_ok = i2cLookup(addr) ;   
   
    if (rc_ok) {
      I2C_devices[i].isAvailable = true ;
    } 
    
    const char* statusIcon = rc_ok ? "✅" : "❌";
    const char* statusText = rc_ok ? "available" : "not found";
    logfTask(LOG_INFO,"::initSensors %s I2C device [%s] %s at addr: [0x%02X]",statusIcon, name, statusText, addr);
  }

  // Sensors DataModel init  
  sensorsModel.load() ; 

}

bool i2cLookup(uint8_t address) {
  TwoWire_I2C1.beginTransmission(address);
  uint8_t error = TwoWire_I2C1.endTransmission();
  return (error == 0);
}

void startSensorsTask()
{
  xTaskCreate( taskSensors, "Sensors", 4096, NULL, PRIORITY_LOW, NULL);
}


// =========================================================
//   -- RTOS Task -- 
//   Read BH1750 Ambient light sensor
//   Read FS400-SHT31 Temperature Humidity sensor
// =========================================================
void taskSensors(void *pvParameters)  
{

  logfTask(LOG_INFO,"▶️ started.");
  
  xQueueOverwrite(queueSensorDataModel, &sensorsModel);

  if (lightSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, BH1750_addr , &TwoWire_I2C1)) {
        logfTask(LOG_INFO,"⚙️ BH1750 Light sensor Configuration done." ) ;
  } else {
        logfTask(LOG_INFO,"🟡 BH1750 Light Sensor Warning Configuration issue." ) ;
  }

  if (!plantAirSensor.begin(SHT31_addr)) {
    logfTask(LOG_INFO,"🟡 SHT31 Temp. Humidity Sensor Warning !! not found.");
  }

  if (!exteriorAirSensor.begin(BME280_addr,&TwoWire_I2C1)) 
  {
    logfTask(LOG_INFO,"🟡 BME280 Temp. Humidity  Pressur Sensor Warning !! not found.");
  } else 
  { //-- Weather Station Scenario --     
    exteriorAirSensor.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF   );
    logfTask(LOG_INFO,"🟢 BME280 Sensor 'Weather Station' MODE_FORCED setup ready.");
  
  }
  
  // ----------- Loop --------------
  while (true) 
  {

  //logfTask(LOG_DEBUG,"⚙️ DEBUG [sensorsModel] Properties: %s ", sensorsModel.toJson().c_str()  );

  // -------- Ambient Light BH1750 --------- 
    if (lightSensor.measurementReady())
    {
      int currentValue  = lightSensor.readLightLevel();
      if  ( abs( sensorsModel.lightSensor.light  -  currentValue ) > 10 )  
      {
        logfTask(LOG_INFO,"%-32s %6d lx --> %6d lx sampling: %d sec.", "lightSensor:", sensorsModel.lightSensor.light ,  currentValue, cmdModel.samplingSensors);
        sensorsModel.lightSensor.light  = currentValue ;
        counter++ ;  
      }
    }
    else {
      logfTask(LOG_WARNING,"🟡 BH1750 LightSensor is not ready." ) ;
    }

    // --------- SHT31 Temperature Humidity
    float currentTemperatureValue  = plantAirSensor.readTemperature();
    if (! isnan(currentTemperatureValue)) // check if 'is not a number'
    {
      float currentPlantTemperatureValue  = round(plantAirSensor.readTemperature() * 10.0f) / 10.0f;

      if (fabs(sensorsModel.plantAirSensor.temperature  - currentPlantTemperatureValue) > 0.25f )  
      {
        logfTask(LOG_INFO,"%-32s %6.1f°   --> %6.1f°   sampling: %d sec.","plantAirSensor Temperature:" , sensorsModel.plantAirSensor.temperature ,  currentPlantTemperatureValue , cmdModel.samplingSensors);
        sensorsModel.plantAirSensor.temperature  = currentPlantTemperatureValue ;
        counter++ ;  
      }
    }   
    else 
    { 
      logfTask(LOG_WARNING,"🟡 WARNING Failed to read SHT31 temperature.");
    }

    vTaskDelay(pdMS_TO_TICKS(250)); // smooth before read again  
    
    float value_f  = plantAirSensor.readHumidity();
    if (! isnan(value_f) ) 
    {  
      int currentHumidityValue = (int)round(value_f) ; 
      if (  abs (sensorsModel.plantAirSensor.humidity - currentHumidityValue ) >=  2 ) 
      {
        logfTask(LOG_INFO,"%-32s %6d%%   --> %6d%%   sampling: %d sec.","plantAirSensor % Humidity:" , sensorsModel.plantAirSensor.humidity ,  currentHumidityValue , cmdModel.samplingSensors);
        sensorsModel.plantAirSensor.humidity  = currentHumidityValue ;
        counter++ ;  
      }  
    } else 
    { 
      logfTask(LOG_WARNING,"🟡 Failed to read SHT31 humidity.");
    }
  // -------- BME280  --------- 
    exteriorAirSensor.takeForcedMeasurement(); 
    float currentExtTemperatureValue  = round(exteriorAirSensor.readTemperature() * 10.0f) / 10.0f;
    if (! isnan(currentExtTemperatureValue)) // check if 'is not a number'
    {
      if (fabs(sensorsModel.exteriorAirSensor.temperature  - currentExtTemperatureValue) > 0.25f )  
      {
        logfTask(LOG_INFO,"%-32s %6.1f°   --> %6.1f°   sampling: %d sec.","exteriorAirSensor Temperature:" , sensorsModel.exteriorAirSensor.temperature ,  currentExtTemperatureValue , cmdModel.samplingSensors);
        sensorsModel.exteriorAirSensor.temperature  = currentExtTemperatureValue ;
        counter++ ;  
      }
    }   
    else 
    { 
      logfTask(LOG_WARNING,"🟡 Failed to read BME280 temperature.");
    }

    vTaskDelay(pdMS_TO_TICKS(250)); // smooth before read again  

    float currentExtHumidityValue  = exteriorAirSensor.readHumidity();
    if (! isnan(currentExtHumidityValue) ) 
    {  
      int value_i = (int)round(currentExtHumidityValue) ; 
      if  (abs( sensorsModel.exteriorAirSensor.humidity  - value_i)  >= 2  ) 
      {
        logfTask(LOG_INFO,"%-32s %6d%%   --> %6d%%   sampling: %d sec.","exteriorAirSensor % Humidity:" , sensorsModel.exteriorAirSensor.humidity ,  value_i , cmdModel.samplingSensors);
        sensorsModel.exteriorAirSensor.humidity  = value_i ;
        counter++ ;  
      }  
    } else 
    { 
      logfTask(LOG_WARNING,"🟡 Failed to read BME280 humidity.");
    }

    vTaskDelay(pdMS_TO_TICKS(250)); // smooth before read again  

    float currentExtPressureValue  = exteriorAirSensor.readPressure();
    if (! isnan(currentExtPressureValue)) // check if 'is not a number'
    {
      float value_f = round((currentExtPressureValue / 100.0f) * 10.0f) / 10.0f; // hPA
      if (fabs(sensorsModel.exteriorAirSensor.pressure  - value_f ) > 0.25f )  
      {
        logfTask(LOG_INFO,"%-32s %6.1f lx --> %6.1f lx sampling: %d sec.","exteriorAirSensor Pressure:" , sensorsModel.exteriorAirSensor.pressure ,  value_f  , cmdModel.samplingSensors);
        sensorsModel.exteriorAirSensor.pressure  = value_f ;
        counter++ ;  
      }
    }   
    else 
    { 
      logfTask(LOG_WARNING,"🟡 Failed to read BME280 temperature.");
    }

    // -- DataModel Update 
    if (counterBackup != counter ) 
    {
      xQueueOverwrite(queueSensorDataModel, &sensorsModel);
      logfTask(LOG_DEBUG,"OverWrite Rtos sensors_dataModel queue ID: %03d" , counter ) ; 
      sensorsModel.modelDirty  = true ; 
      sensorsModel.save() ; 
      counterBackup = counter ; 
    }
    vTaskDelay(pdMS_TO_TICKS(cmdModel.samplingSensors * 1000));
  } // end While 
}

/* --- for I2C DEBUG purpose --- 
String scanI2C(TwoWire &wireBus) {
    String result = "I2C Scan Results: ";
    uint8_t count = 0;

    for (uint8_t addr = 1; addr < 128; addr++) {  // I2C addresses 0x01 to 0x7F
        wireBus.beginTransmission(addr);
        uint8_t error = wireBus.endTransmission();

        if (error == 0) {
            result += "[0x";
            if (addr < 16) result += "0";  // pour toujours 2 chiffres
            result += String(addr, HEX);
            result += "] ";
            count++;
        } 
        // On ignore les autres codes d'erreur (1 = NACK, 2 = bus error)
    }

    if (count == 0) {
        result += "No I2C devices found.\n";
    }

    return result;
}
*/

/*
void initSensors() {

  Serial.println(scanI2C(TwoWire_I2C1)) ; 

}
*/
