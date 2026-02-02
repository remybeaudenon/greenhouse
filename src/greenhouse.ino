/*
  Connected Greenhouse 

   MCU : Heltec  [Wifi LoRa 32(V3)]  

   preferences: c:\Users\remyb\Google Drive\MyProjects\Ecole IOT Polytech\Kit Etudiant\HELTECV3  
   additional borad : https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp2‌​32_index.json
   Libraries  HELTECV3\libraries\Rotary  url=https://github.com/skathir38/Rotary 
                                \BH1750  url=https://github.com/claws/BH1750 ( use for dedicated Physical I2C Wire )
                                \Heltec ESP32 Dev-Boards    url=https://github.com/HelTecAutomation/Heltec_ESP32.git
                            NOTA: Remove BH1750 to avoid conflict       
  VERSION 1.0.1 
  2/02/2026 
*/
#include <Arduino.h>
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include <Rotary.h>
#include <BH1750.h>
#include "gpio.h" 
// =========================================================
//                     OLED HELTEC
// =========================================================
static SSD1306Wire display(
  0x3c,
  500000,
  SDA_OLED,
  SCL_OLED,
  GEOMETRY_128_64,
  RST_OLED
);

// =========================================================
//                     GPIO
// =========================================================
//#define GPIO_BOARD_LED     LED_BUILTIN
//#define GPIO_ENC_CLK 20
//#define GPIO_ENC_DT  19
//#define GPIO_BOARD_BP 0

// =========================================================
//                     BH1750 SUR BUS DÉDIÉ
// =========================================================
//#define GPIO_BH1750_SDA 47
//#define GPIO_BH1750_SCL 48

TwoWire I2CBH1750(1);        // 👉 TON bus séparé
BH1750 lightMeter(0x23);

// =========================================================
//                     RTOS
// =========================================================

typedef enum {
  EVT_NONE,
  EVT_UP,
  EVT_DOWN,
  EVT_SELECT,
  EVT_SELECT_LONG
} EncoderEvent_t;

QueueHandle_t encoderQueue;
QueueHandle_t queueLux;

// =========================================================
//                     ROTARY
// =========================================================
Rotary encoder(GPIO_ENC_DT, GPIO_ENC_CLK);

#define DIR_NONE 0x00
#define DIR_CW   0x10
#define DIR_CCW  0x20
// =========================================================
//                     MENU
// =========================================================
enum AppMode {
  FAN_STOP,
  FAN_ON,
  HEATER_ON,
  HEATER_OFF
};

AppMode appMode = HEATER_OFF;

struct MenuItem {
  const char* label;
  AppMode id;
};

MenuItem menuItems[] = {
  { "FAN STOP",    FAN_STOP },
  { "FAN ON", FAN_ON },
  { "HEATER ON",  HEATER_ON },
  { "HEATER OFF",   HEATER_OFF }
};

const uint8_t MENU_COUNT =
  sizeof(menuItems)/sizeof(menuItems[0]);

volatile uint8_t menuIndex = 3;

// =========================================================
//               STRUCT CAPTEUR
// =========================================================
typedef struct {
  float lux;
} bh1750_data_t;

bh1750_data_t lastLux = {0};


// =========================================================
//               UTILITAIRES
// =========================================================

uint8_t getActiveIndex() {
  for (uint8_t i = 0; i < MENU_COUNT; i++) {
    if (menuItems[i].id == appMode)
      return i;
  }
  return 0;
}

// =========================================================
//               AFFICHAGE OLED
// =========================================================

void drawMenu() {

  bh1750_data_t luxLocal;

  if (xQueuePeek(queueLux, &luxLocal, 0) == pdTRUE) {
    lastLux = luxLocal;
  }

  display.clear();

  // ----- BARRE STATUS -----
  display.setColor(WHITE);
  display.fillRect(0, 0, 128, 14);

  display.setColor(BLACK);
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  char buf[30];
  sprintf(buf, "Ambient light: %.0f lx", lastLux.lux);
  display.drawString(4, 2, buf);

  // ----- ZONE MENU -----
  display.setColor(WHITE);
  display.drawRect(0, 14, 128, 50);

  const uint8_t visibleItems = 3;
  int start = (menuIndex >= visibleItems)
              ? menuIndex - visibleItems + 1
              : 0;

  for (uint8_t i = 0; i < visibleItems; i++) {

    uint8_t idx = start + i;
    if (idx >= MENU_COUNT) break;

    uint8_t y = 18 + i * 14;

    if (idx == menuIndex) {
      display.fillRect(5, y, 118, 12);
      display.setColor(BLACK);
    } else {
      display.setColor(WHITE);
    }

    display.drawString(10, y, menuItems[idx].label);
  }

  // Etoile mode actif
  display.setColor(WHITE);

  uint8_t activeIndex = getActiveIndex();

  if (activeIndex >= start &&
      activeIndex < start + visibleItems) {

    uint8_t y = 18 + (activeIndex - start) * 14;

    if (activeIndex == menuIndex) {
      display.setColor(BLACK);
      display.drawString(105, y, "*");
      display.setColor(WHITE);
    } else {
      display.drawString(105, y, "*");
    }
  }

  display.display();
}

// =========================================================
//                     TÂCHES
// =========================================================

// -------- Tâche BH1750 --------
void taskReadBH1750(void *pvParameters) {
    bh1750_data_t data;
    float lastLux = -1; // valeur initiale impossible pour déclencher le premier affichage

    //vTaskDelay(pdMS_TO_TICKS(30000));

    for (;;) {
        data.lux = lightMeter.readLightLevel();

        xQueueOverwrite(queueLux, &data);

        if (data.lux != lastLux) {
            Serial.print("Lux: ");
            Serial.println(data.lux);
            lastLux = data.lux;
        }

        if (data.lux = -2 ) 
          lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE,0x23,&I2CBH1750) ; 


        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


// -------- Tâche LED --------


void TaskPLC(void *pvParameters) {

  // ------- INPUT --------------
  bh1750_data_t luxLocal;
  //appMode 
  
  // ------- LOGIC --------------
  
  while (true) {

    // ---- INPUT --------- 
    if (xQueuePeek(queueLux, &luxLocal, 0) == pdTRUE) {
      lastLux = luxLocal;
    } 
    // appMode Global static variable 

    switch (appMode) {

      case FAN_ON:
        digitalWrite(GPIO_FAN, HIGH); 
        analogWrite(GPIO_PWM_HEATER, 0);
        break;

      case FAN_STOP:
        digitalWrite(GPIO_FAN, LOW); 
        analogWrite(GPIO_PWM_HEATER, 0);
        break;

      case HEATER_ON:
        {
         float lux = lastLux.lux ;    
         // Saturation des bornes d'entrée
         if (lux <= 0)
          lux = 0;
         if (lux >= 1000)
           lux = 255;

         // Mise à l'échelle linéaire
         uint8_t power = static_cast<uint8_t>((lux * 255) / 1000);
         analogWrite(GPIO_PWM_HEATER, power);
         digitalWrite(GPIO_FAN, LOW); 
        }
        break;

      case HEATER_OFF:

        analogWrite(GPIO_PWM_HEATER, 0 );
        digitalWrite(GPIO_FAN, LOW); 
        break;

    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// -------- Tâche ENCODEUR --------
void encoderTask(void *parameter) {
    pinMode(GPIO_BOARD_BP, INPUT_PULLUP);

    bool lastReadState = HIGH;
    bool stableState   = HIGH;
    uint32_t btnPressTime = 0;
    const uint16_t DEBOUNCE_MS = 50;
    uint32_t lastDebounceTime = 0;

    // Pour suivre la position précédente
    int lastPosition = encoder.getPosition();

    while (true) {
        // Mets à jour l'encodeur
        encoder.loop();

        // Vérifie la rotation
        int currentPosition = encoder.getPosition();
        int diff = currentPosition - lastPosition;

        if (abs(diff)  > 2 ) {  // 1 cran 
            EncoderEvent_t evt = (diff > 0) ? EVT_DOWN : EVT_UP;
            xQueueSend(encoderQueue, &evt, 0);

            // Actualise la position
            lastPosition = currentPosition;
        }

        // Gestion du bouton avec debounce
        bool reading = digitalRead(GPIO_BOARD_BP);

        if (reading != lastReadState) {
            lastDebounceTime = millis();
        }

        if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
            if (stableState == HIGH && reading == LOW) {
                btnPressTime = millis();
            }

            if (stableState == LOW && reading == HIGH) {
                uint32_t pressDuration = millis() - btnPressTime;
                EncoderEvent_t evt = (pressDuration >= 1000)
                                     ? EVT_SELECT_LONG
                                     : EVT_SELECT;
                xQueueSend(encoderQueue, &evt, 0);
            }

            stableState = reading;
        }

        lastReadState = reading;

        // Petite pause FreeRTOS
        vTaskDelay(pdMS_TO_TICKS(3));
    }
}

// -------- Tâche MENU --------
void menuTask(void *parameter) {

  EncoderEvent_t evt;

  drawMenu();

  uint32_t lastRefresh = 0;

  while (true) {

    if (xQueueReceive(encoderQueue,
                      &evt,
                      pdMS_TO_TICKS(50))) {

      switch (evt) {

        case EVT_UP:
          if (menuIndex > 0) menuIndex--;
          break;

        case EVT_DOWN:
          if (menuIndex < MENU_COUNT - 1)
            menuIndex++;
          break;

        case EVT_SELECT:
          appMode = menuItems[menuIndex].id;
          break;

        default:
          break;
      }

      drawMenu();
    }

    // 👉 RAFRAICHISSEMENT DYNAMIQUE
    if (millis() - lastRefresh > 1000) {
      drawMenu();
      lastRefresh = millis();
    }
  }
}


// =========================================================
//                     SETUP
// =========================================================

void setup() {

  Serial.begin(115200);
  delay(500);

  initGPIO();

  encoder.begin(GPIO_ENC_DT, GPIO_ENC_CLK);

  display.init();
  display.clear();
  display.display();

  // ===== TON INIT QUI MARCHE =====
  I2CBH1750.begin(GPIO_BH1750_SDA, GPIO_BH1750_SCL);

  if (lightMeter.begin(
        BH1750::CONTINUOUS_HIGH_RES_MODE,
        0x23,
        &I2CBH1750)) {

    Serial.println("BH1750 OK");
  } else {
    Serial.println("WARNING !!! BH1750 non détecté !");
  }

  encoderQueue =
    xQueueCreate(20, sizeof(EncoderEvent_t));

  queueLux =
    xQueueCreate(1, sizeof(bh1750_data_t));

  xTaskCreate(TaskPLC, "TaskPLC", 2048, NULL, 1, NULL);

  xTaskCreatePinnedToCore(
      encoderTask, "EncoderTask",
      4096, NULL, 2, NULL, 0);

  xTaskCreatePinnedToCore(
      menuTask, "MenuTask",
      4096, NULL, 2, NULL, 1);

  xTaskCreate(
      taskReadBH1750, "BH1750",
      4096, NULL, 1, NULL);

  Serial.println("Systeme complet prêt");
  display.displayOn();
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}