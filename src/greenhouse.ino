/*
  Connected Greenhouse 

   MCU Heltec V3 LoRa (32) 

   preferences: c:\Users\remyb\Google Drive\MyProjects\Ecole IOT Polytech\Kit Etudiant\HELTECV3  
   additional borad : https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp2‌​32_index.json
   Libraries  HELTECV3\libraries\Rotary  url=https://github.com/skathir38/Rotary 
                                \BH1750  url=https://github.com/claws/BH1750 ( use for dedicated Physical I2C Wire )
                                \Heltec ESP32 Dev-Boards    url=https://github.com/HelTecAutomation/Heltec_ESP32.git
                            NOTA: Remove BH1750 to avoid conflict       
  VERSION 1.0.0 
  2/02/2026 
*/
#include <Arduino.h>
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include <Rotary.h>
#include <BH1750.h>

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
#define LED_PIN     LED_BUILTIN
#define ENCODER_CLK 20
#define ENCODER_DT  19
#define ENCODER_BTN 0

// =========================================================
//                     BH1750 SUR BUS DÉDIÉ
// =========================================================
#define SDA_BH1750 47
#define SCL_BH1750 48

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
Rotary encoder(ENCODER_DT, ENCODER_CLK);

#define DIR_NONE 0x00
#define DIR_CW   0x10
#define DIR_CCW  0x20
// =========================================================
//                     MENU
// =========================================================
enum LedMode {
  LED_OFF,
  LED_ON,
  LED_BLINK,
  LED_TRIM
};

LedMode ledMode = LED_TRIM;

struct MenuItem {
  const char* label;
  LedMode id;
};

MenuItem menuItems[] = {
  { "LED OFF",    LED_OFF },
  { "LED ON",     LED_ON },
  { "LED BLINK",  LED_BLINK },
  { "LED TRIM",   LED_TRIM }
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
    if (menuItems[i].id == ledMode)
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

  for (;;) {

    data.lux = lightMeter.readLightLevel();

    xQueueOverwrite(queueLux, &data);

    Serial.print("Lux: ");
    Serial.println(data.lux);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}


// -------- Tâche LED --------
void TaskLED(void *pvParameters) {

  pinMode(LED_PIN, OUTPUT);

  while (true) {

    switch (ledMode) {

      case LED_ON:
        analogWrite(LED_PIN, 255);
        break;

      case LED_OFF:
        analogWrite(LED_PIN, 0);
        break;

      case LED_BLINK:
        analogWrite(LED_PIN, 255);
        vTaskDelay(pdMS_TO_TICKS(500));
        analogWrite(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        continue;

      case LED_TRIM:
        
        for (int duty = 0; duty <= 255 && ledMode == LED_TRIM; duty++) {
          analogWrite(LED_PIN, duty);
          vTaskDelay(pdMS_TO_TICKS(5));
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        
        for (int duty = 255; duty >= 0 && ledMode == LED_TRIM; duty--) {
          analogWrite(LED_PIN, duty);
          vTaskDelay(pdMS_TO_TICKS(5));
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        break;

    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// -------- Tâche ENCODEUR --------
void encoderTask(void *parameter) {
    pinMode(ENCODER_BTN, INPUT_PULLUP);

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
            Serial.println(diff) ;
            EncoderEvent_t evt = (diff > 0) ? EVT_DOWN : EVT_UP;
            xQueueSend(encoderQueue, &evt, 0);

            // Actualise la position
            lastPosition = currentPosition;
        }

        // Gestion du bouton avec debounce
        bool reading = digitalRead(ENCODER_BTN);

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
          ledMode = menuItems[menuIndex].id;
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

  encoder.begin(ENCODER_DT, ENCODER_CLK);

  display.init();
  display.clear();
  display.display();

  // ===== TON INIT QUI MARCHE =====
  I2CBH1750.begin(SDA_BH1750, SCL_BH1750);

  if (lightMeter.begin(
        BH1750::CONTINUOUS_HIGH_RES_MODE,
        0x23,
        &I2CBH1750)) {

    Serial.println("BH1750 OK");
  } else {
    Serial.println("BH1750 non détecté !");
  }

  encoderQueue =
    xQueueCreate(20, sizeof(EncoderEvent_t));

  queueLux =
    xQueueCreate(1, sizeof(bh1750_data_t));

  xTaskCreate(TaskLED, "TaskLED", 2048, NULL, 1, NULL);

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