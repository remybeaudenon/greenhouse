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

char tmpBuffer[128];  

// ----------------------------------------

sensors_dataModel_t  sensor_dataModel = { 
    .lux = 0.0f,
    .humidity = 0,
    .temperature = 0.0f,
    .counter = 0 
};


int sampling = 10 ;  // every 10 sec. 

// -- Functions 
void initSensors() {
  
//  char tmpBuffer[128];  

  TwoWire_I2C1.begin(GPIO_I2C_1_SDA, GPIO_I2C_1_SCL);

  for (int i = 0; i < DEVICE_COUNT; i++) {
    uint8_t addr = I2C_devices[i].addr;
    const char *name = I2C_devices[i].label;
  
    bool rc_ok = i2cLookup(addr) ;   
    if (rc_ok) {
      snprintf(tmpBuffer, sizeof(tmpBuffer), "Main::initSensors ✅ I2C device [%s] available at addr: [0x%02X] ", name, addr );
      I2C_devices[i].isAvailable = true ;
      Serial.println(tmpBuffer) ; 
    } else {
      snprintf(tmpBuffer, sizeof(tmpBuffer), "Main::initSensors ❌ I2C device [%s] not found at addr: [0x%02X] ", name, addr );
      Serial.println(tmpBuffer) ; 
    }
   
  }
}

bool i2cLookup(uint8_t address) {
  TwoWire_I2C1.beginTransmission(address);
  uint8_t error = TwoWire_I2C1.endTransmission();
  return (error == 0);
}

void startSensorsTask()
{
  xTaskCreate( taskSensors, "Sensors ", 4096, NULL, PRIORITY_LOW, NULL);
}


// =========================================================
//   -- RTOS Task -- 
//   Read BH1750 Ambient light sensor
//   Read FS400-SHT31 Temperature Humidity sensor
// =========================================================
void taskSensors(void *pvParameters)  
{
  sensors_dataModel_t sensor_dataModel_backup = sensor_dataModel;
  char tmpBuffer[128];  //  temporary tmpBufferfer

  String taskName = pcTaskGetName(NULL) ; 
  logTask(taskName,"▶️ started.");

  // queue initialisation with empty data   
  xQueueOverwrite(queueSensorDataModel, &sensor_dataModel);
 
  if (lightSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, BH1750_addr , &TwoWire_I2C1)) {
        logTask(taskName, "BH1750 Configuration done." ) ;
  } else {
        logTask(taskName, "BH1750 Warning Configuration issue." ) ;
  }

  if (!tempHumSensor.begin(SHT31_addr)) {
    logTask(taskName, "SHT31 Warning !! not found.");
  }

  // ----------- Loop --------------
  while (true) 
  {

  // ---------Ambient Light 
    if (lightSensor.measurementReady())
    {
      int currentValue  = lightSensor.readLightLevel();
      if (sensor_dataModel_backup.lux  != currentValue ) 
      {
        snprintf(tmpBuffer, sizeof(tmpBuffer), "%12s %4d --> %4d sampling: %d sec.", "Lux:", sensor_dataModel_backup.lux ,  currentValue, sampling);
        logTask(taskName, tmpBuffer ) ;
        sensor_dataModel.lux = currentValue ;
        sensor_dataModel.counter++ ;  
      }
    }
    else {
      logTask(taskName, "LightSensor Not ready." ) ;
    }

    // --------- SHT31 Temperature Humidity
    float currentTemperatureValue  = tempHumSensor.readTemperature();
    if (! isnan(currentTemperatureValue)) 
    {  // check if 'is not a number'
      if (fabs(sensor_dataModel_backup.temperature  - currentTemperatureValue) > 0.10f )  
      {
        snprintf(tmpBuffer, sizeof(tmpBuffer), "%12s %4.1f --> %3.1f sampling: %d sec.","Temperature:" , sensor_dataModel_backup.temperature ,  currentTemperatureValue , sampling);
        logTask(taskName, tmpBuffer ) ;
        sensor_dataModel.temperature  = currentTemperatureValue ;
        sensor_dataModel.counter++ ;  
      }
    }   
    else 
    { 
      logTask(taskName, "WARNING Failed to read temperature.");
    }

    vTaskDelay(pdMS_TO_TICKS(250)); // smooth before read again  
    
    float value_f  = tempHumSensor.readHumidity();
    if (! isnan(value_f) ) 
    {  
      int currentHumidityValue = (int)round(value_f) ; 
      if (sensor_dataModel_backup.humidity  != currentHumidityValue ) 
      {
        snprintf(tmpBuffer, sizeof(tmpBuffer), "%12s %4d --> %4d sampling: %d sec.","% Humidity:" , sensor_dataModel_backup.humidity ,  currentHumidityValue , sampling);
        logTask(taskName, tmpBuffer ) ;
        sensor_dataModel.humidity  = currentHumidityValue ;
        sensor_dataModel.counter++ ;  
      }  
    } else 
    { 
      logTask(taskName, "WARNING Failed to read humidity.");
    }
    
    // -- DataModel Update 
    if (sensor_dataModel_backup.counter != sensor_dataModel.counter ) 
    {
      xQueueOverwrite(queueSensorDataModel, &sensor_dataModel);
      snprintf(tmpBuffer, sizeof(tmpBuffer), "OverWrite Rtos sensors_dataModel queue ID: %03d" ,sensor_dataModel.counter ) ; 
      logTask(taskName, tmpBuffer ) ;
      sensor_dataModel_backup = sensor_dataModel ; 
    }
    
    vTaskDelay(pdMS_TO_TICKS(sampling * 1000));
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
