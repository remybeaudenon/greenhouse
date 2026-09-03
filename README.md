# greenhouse
Polytech 45 Connected green house  -  MCU Heltec loRa V3  Ambient light BH1750

Project Objective
  The goal of this project is to design a connected IoT system that measures ambient light and controls a resistive heater in order to simulate solar irradiation.

For our laboratory experiences the relationship between solar irradiance and the energy supplied to the greenhouse must be as proportional as possible.

System Features
Ambient light monitoring to regulate the heating source.
Temperature sensor for heat regulation and frost protection.
Humidity sensor to control the environment through ventilation
Local display showing system status and fault information.
Push button to acknowledge system faults .
      Note : The greenhouse is already equipped with a 12 Volts 0.45 Watt fan. and heating element 10 ohms 25 Watts without its power supply unit.
Greenhouse picture
Remote Supervision
     A dedicated remote supervision software enables real-time monitoring, control, and data visualization. 
     It also provides access to historical trends and weather forecasts for the next seven days. 
Energy Constraints
     Since the system is powered by a 12V solar supply, it is essential to implement strategies that minimize energy consumption while maintaining reliable operation. Based on the rought power need we envisage to install a  battery lead-acid : E=12V×8Ah=96Wh with a 25 Watts Solar panel .

NE FONCTIONNE PAS !! 
greenhouse/                  <- racine du projet
│
├─ src/                      <- tous les fichiers header sources
│   ├─ blinker.h             
│   ├─ displayCtrl.h                
│   ├─ globals.h                
│   ├─ gpio.h                
│   ├─ logger.h                
│   ├─ models.h            
│   ├─ mqtt.h            
│   ├─ plc.h            
│   ├─ rotaryEncoder.h            
│   ├─ RTOSQueues.h            
│   ├─ sensors.h            
│
├─ /                      <- tous les fichiers sources
│   ├─ greenhouse.ino        <- setup minimal, lance les tasks
│   ├─ other packages.cpp
│
└─ (optionnel) libraries/    <- si tu as des libs locales ou modifiées

Polytech Connected Greenhouse Project 02/2026   
   MCU : Heltec  [Wifi LoRa 32(V3)]  

   IIDE : Aduino IDE 2.3.10
   preferences: c:\Users\remyb\Google Drive\MyProjects\Ecole IOT Polytech\MCU\HELTECV3  
   additional borad : https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp2‌​32_index.json
   Libraries  HELTECV3\libraries\Rotary                     url=https://github.com/skathir38/Rotary 
                                \BH1750                     url=https://github.com/claws/BH1750 ( use for dedicated Physical I2C Wire )
                                \SHT31                      url=https://github.com/adafruit/Adafruit_SHT31
                                \Heltec ESP32 Dev-Boards    url=https://github.com/HelTecAutomation/Heltec_ESP32.git
                                 NOTA: Remove BH1750.cpp .h  to avoid conflict       
                                \HT_SSD1306Wire
  -- Keys feature --  
  FreeRTOS (tasks, queues)
  I2C secondaire
  OLED Heltec spécifique
  BH1750
  FS400-SHT31 
  encodeur rotatif
  logique “PLC”

  logs: Serial Output 155200 bauds 
  