/*
  Polytech Connected Greenhouse Project 02/2026   
# ----------------------------------------------------------
  The project aims to design and implement an IoT system 
  for the control, supervision, and data utilization from 
  connected sensors, in order to monitor and optimize the 
  operation of an environment's equipment.
# ----------------------------------------------------------

   MCU : Heltec  [Wifi LoRa 32(V3)]  
   IIDE : Aduino IDE 2.3.10
   preferences: c:\Users\remyb\Google Drive\MyProjects\Ecole IOT Polytech\MCU\HELTECV3  
   additional borad : https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp2‌​32_index.json
   Libraries  HELTECV3\libraries\Rotary                       url=https://github.com/skathir38/Rotary 
                                \BH1750                       url=https://github.com/claws/BH1750 ( use for dedicated Physical I2C Wire )
                                \SHT31                        url=https://github.com/adafruit/Adafruit_SHT31
                                \Adafruit_BME280_LibrarySHT31 url=https://github.com/adafruit/Adafruit_BME280_Library
                                \Heltec ESP32 Dev-Boards      url=https://github.com/HelTecAutomation/Heltec_ESP32.git
                                 NOTA: Remove BH1750.cpp .h  to avoid conflict after library update ! .         
                                \HT_SSD1306Wire
  -- Keys feature --  
  FreeRTOS (tasks, queues)
  BUS: I2C ,I2CWIRE 
  Display : OLED Heltec spécifique
  Sensors : BH1750 ,  FS400-SHT31, BME280 
  Rotary Enoder 
  logical “PLC”
  Logs: Serial Output 155200 bauds , Loki over mqtt 'greenhouse/log'   
    
  VERSION ==> see Line 44
  clean up folder ==> C:\Users\remyb\AppData\Local\arduino\sketches\669A8ABA8C4B022958FBAB0FD3A038F8\sketch\
*/
#include "src\gpio.h" 
#include "src\globals.h"
#include "src\models.h"
#include "src\RTOSQueues.h"
#include "src\blinker.h"
#include "src\plc.h"
#include "src\sensors.h"
#include "src\rotaryEncoder.h"
#include "src\displayCtrl.h"
#include "src\mqtt.h"

#include <Arduino.h>

//#include <RadioLib.h>
//#include <esp_now.h>


const char* APP_VERSION = "VERSION 3.2.1";

//init for future LoRa use ... 
//SX1262 radio = new Module(8, 14, 12, 13);

void setup() {

  Serial.begin(115200);
  delay(500);
  
  logfTask(LOG_INFO,"##########################################################") ; 
  logfTask(LOG_INFO,"# ---- greenhouse.ino started Version: %7s --- #" , APP_VERSION) ; 
  logfTask(LOG_INFO,"##########################################################") ; 

  // -- RTOS Queues   ---- 
  createQueues();
  logfTask(LOG_INFO,"======> GREEN HOUSE.INO STARTED  Release: %7s <====== " , APP_VERSION) ; 

  initGPIO();
  
  // -- DataModel Init ---- 
  cmdModel.load() ; 

  // -- Rotary Encode -------- 
  initRotaryEncoder(); 
  startRotaryEncoderTask() ; 

  // -- Display LCD   ---- 
  initDisplayCtrl(); 
  startDisplayCtrlTask(); 
  
  // -- Sensors ---- 
  initSensors(); 
  startSensorsTask() ;
  
  initBlinker() ; 
  startBlinkerTask() ; 

  // -- PLC Logic Control ---- 
  initPLC(); 
  startPLCTask();  

  // -- WiFi MQTT ---- 
  initMqtt(); 
  startMqttTask();  

  logfTask(LOG_INFO, "🟢 Setup Completed !! 🟢 ");

}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
