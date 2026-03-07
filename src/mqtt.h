#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "globals.h"
#include "RTOSQueues.h"
#include "gpio.h"
#include "logger.h"

#include <WiFi.h>

#include <PubSubClient.h>

#include <ArduinoJson.h>

// == XQueue record  
#define MQTT_MAX_TOPIC_LEN    64
#define MQTT_MAX_PAYLOAD_LEN  128
typedef struct
{
    char topic[MQTT_MAX_TOPIC_LEN];
    uint8_t payload[MQTT_MAX_PAYLOAD_LEN];
    uint16_t length;
    uint32_t timestamp;  // utile pour debug ou timeout
} MqttMessage_t;

// == MCU States   
enum McuSystemState {
  MCU_WIFI_CONNECTING,
  MCU_MQTT_CONNECTING,
  MCU_MQTT_CONNECTED,
  MCU_RUNNING
};

void initMqtt(void) ;
void startMqttTask(void);

void taskMqtt(void *pvParameters);

void applyForceDigitalGPIO(uint8_t gpio, bool force, uint8_t value);
void applyForcePWMGPIO(uint8_t gpio, bool force, uint8_t value) ; 



