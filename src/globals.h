#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Rotary.h>
#include <BH1750.h>
#include "HT_SSD1306Wire.h"
#include "gpio.h"   // <-- inclut toutes les pins et initGPIO()

// ================== I2C BH1750 ==================
extern TwoWire I2CBH1750;    // Bus séparé pour BH1750
extern BH1750 lightMeter;

// ================== OLED ==================
extern SSD1306Wire display;

// ================== RTOS ==================
typedef enum { EVT_NONE, EVT_UP, EVT_DOWN, EVT_SELECT, EVT_SELECT_LONG } EncoderEvent_t;
extern QueueHandle_t encoderQueue;
extern QueueHandle_t queueLux;

// ================== MENU ==================
enum LedMode { LED_OFF, LED_ON, LED_BLINK, LED_TRIM };
extern LedMode ledMode;
extern volatile uint8_t menuIndex;

struct MenuItem {
    const char* label;
    LedMode id;
};
extern MenuItem menuItems[];
extern const uint8_t MENU_COUNT;

// ================== BH1750 struct ==================
typedef struct { 
    float lux; 
} bh1750_data_t;
extern bh1750_data_t lastLux;

// ================== ROTARY ==================
extern Rotary encoder;
