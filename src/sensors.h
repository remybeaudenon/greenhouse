#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "globals.h"
#include "RTOSQueues.h"
#include <BH1750.h>
#include <Adafruit_SHT31.h>
#include "Wire.h" 

#include "gpio.h"
#include "logger.h"

void initSensors(void);

void startSensorsTask(void);

void taskSensors(void *pvParameters);

bool i2cLookup(uint8_t address) ; 
