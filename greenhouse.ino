#include <RadioLib.h>

/*
  Polytech Connected Greenhouse Project   

   MCU : Heltec  [Wifi LoRa 32(V3)]  

   IIDE : Aduino IDE 2.3.7 
   preferences: c:\Users\remyb\Google Drive\MyProjects\Ecole IOT Polytech\Kit Etudiant\HELTECV3  
   additional borad : https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp2‌​32_index.json
   Libraries  HELTECV3\libraries\Rotary                     url=https://github.com/skathir38/Rotary 
                                \BH1750                     url=https://github.com/claws/BH1750 ( use for dedicated Physical I2C Wire )
                                \SHT31                      url=https://github.com/adafruit/Adafruit_SHT31
                                \Heltec ESP32 Dev-Boards    url=https://github.com/HelTecAutomation/Heltec_ESP32.git
                            NOTA: Remove BH1750 to avoid conflict       
                                \HT_SSD1306Wire
  -- Keys feature --  
  FreeRTOS (tasks, queues)
  I2C secondaire
  OLED Heltec spécifique
  BH1750
  FS400-SHT31 
  encodeur rotatif
  logique “PLC”
    
  VERSION ==> Line 38
  2/02/2026 
  clean up folder ==> C:\Users\remyb\AppData\Local\arduino\sketches\669A8ABA8C4B022958FBAB0FD3A038F8\sketch\
*/
#include "src\gpio.h" 
#include "src\globals.h"
#include "src\RTOSQueues.h"
#include "src\plc.h"
#include "src\sensors.h"
#include "src\rotaryEncoder.h"
#include "src\displayCtrl.h"

const char* APP_VERSION = "VERSION 1.2.0";

//init for future LoRa use ... 
SX1262 radio = new Module(8, 14, 12, 13);

void setup() {

  Serial.begin(115200);
  delay(500);

  Serial.print("--------greenhouse.ino started ");
  Serial.print(APP_VERSION);
  Serial.println("--------");
  
  initGPIO();
  Serial.println ("Main::setup initGPIO() completed.");

  // -- RTOS Queues   ---- 
  createQueues();

  // -- Rotary Encode -------- 
  initRotaryEncoder(); 
  startRotaryEncoderTask() ; 

  // -- Display LCD   ---- 
  initDisplayCtrl(); 
  startDisplayCtrlTask(); 
  
  // -- Sensors ---- 
  initSensors(); 
  startSensorsTask() ;
  
  // -- PLC Logic Control ---- 
  initPLC(); 
  startPLCTask();  

  Serial.println("Main::setup 🟢 completed Ready !! 🟢 ");

}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
