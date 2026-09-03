#include <sys/types.h>
#include "HardwareSerial.h"
#include <stdint.h>
#include "src\sensors.h"

//extern TwoWire TwoWire_I2C1  ;
TwoWire TwoWire_I2C1(1);        // 👉 TON bus séparé

// --- Device list, definition , instance  --  
uint8_t BH1750_addr = 0x23 ;  
uint8_t SHT31_addr  = 0x44 ;  

typedef struct { uint8_t addr; const char *label; bool isAvailable; } I2C_Device_List;
I2C_Device_List I2C_devices[] = {
    { BH1750_addr, "BH1750 Light Sensor" , false  },
    { SHT31_addr, "SHT31 Temp/Humidity" , false }
};
#define DEVICE_COUNT (sizeof(I2C_devices) / sizeof(I2C_devices[0]))

BH1750          lightSensor(BH1750_addr);
Adafruit_SHT31  tempHumSensor(&TwoWire_I2C1);

int counter,counterBackup  = 0 ; 

// -- Functions 
void initSensors() {
  
  TwoWire_I2C1.begin(GPIO_I2C_1_SDA, GPIO_I2C_1_SCL);

  for (int i = 0; i < DEVICE_COUNT; i++) {
    uint8_t addr = I2C_devices[i].addr;
    const char *name = I2C_devices[i].label;
  
    bool rc_ok = i2cLookup(addr) ;   
    if (rc_ok) {
      logfTask("::initSensors ✅ I2C device [%s] available at addr: [0x%02X] ", name, addr );  
      I2C_devices[i].isAvailable = true ;
    } else {
      logfTask("Main::initSensors ❌ I2C device [%s] not found at addr: [0x%02X] ", name, addr );
    }
  }
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

  logfTask("▶️ started.");
  
  xQueueOverwrite(queueSensorDataModel, &sensorsModel);
 
  if (lightSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, BH1750_addr , &TwoWire_I2C1)) {
        logfTask("🟢 BH1750 Light sensor Configuration done." ) ;
  } else {
        logfTask("🟡 BH1750 Light Sensor Warning Configuration issue." ) ;
  }

  if (!tempHumSensor.begin(SHT31_addr)) {
    logfTask("🟡 SHT31 Temp. Humidity Sensor Warning !! not found.");
  }

  // ----------- Loop --------------
  while (true) 
  {

  //logfTask("⚙️  DEBUG [sensorsModel] Properties:  %s ", sensorsModel.toJson().c_str() );

  // -------- Ambient Light BH1750 --------- 
    if (lightSensor.measurementReady())
    {
      int currentValue  = lightSensor.readLightLevel();
      if (sensorsModel.bh1750_light  != currentValue ) 
      {
        logfTask("%12s %4d --> %4d sampling: %d sec.", "Lux:", sensorsModel.bh1750_light ,  currentValue, cmdModel.samplingSensors);
        sensorsModel.bh1750_light  = currentValue ;
        counter++ ;  
      }
    }
    else {
      logfTask("🟡 BH1750 LightSensor Not ready." ) ;
    }

    // --------- SHT31 Temperature Humidity
    float currentTemperatureValue  = tempHumSensor.readTemperature();
    if (! isnan(currentTemperatureValue)) // check if 'is not a number'
    {
      if (fabs(sensorsModel.sht31_temperature  - currentTemperatureValue) > 0.10f )  
      {
        logfTask("%12s %4.1f --> %4.1f sampling: %d sec.","Temperature:" , sensorsModel.sht31_temperature ,  currentTemperatureValue , cmdModel.samplingSensors);
        sensorsModel.sht31_temperature  = currentTemperatureValue ;
        counter++ ;  
      }
    }   
    else 
    { 
      logfTask("🟡 WARNING Failed to read SHT31 temperature.");
    }

    vTaskDelay(pdMS_TO_TICKS(250)); // smooth before read again  
    
    float value_f  = tempHumSensor.readHumidity();
    if (! isnan(value_f) ) 
    {  
      int currentHumidityValue = (int)round(value_f) ; 
      if (sensorsModel.sht31_humidity  != currentHumidityValue ) 
      {
        logfTask("%12s %4d --> %4d sampling: %d sec.","% Humidity:" , sensorsModel.sht31_humidity ,  currentHumidityValue , cmdModel.samplingSensors);
        sensorsModel.sht31_humidity  = currentHumidityValue ;
        counter++ ;  
      }  
    } else 
    { 
      logfTask("🟡 WARNING Failed to read SHT31 humidity.");
    }
    
    // -- DataModel Update 
    if (counterBackup != counter ) 
    {
      xQueueOverwrite(queueSensorDataModel, &sensorsModel);
      logfTask("OverWrite Rtos sensors_dataModel queue ID: %03d" , counter ) ; 
      //sensorsModel_backup = sensorsModel ; 
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
