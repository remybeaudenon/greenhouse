#pragma once

//#include <sys/types.h>
//#include "HardwareSerial.h"
//#include <stdint.h>

//#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <BH1750.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_BME280.h>


#include "Wire.h" 

#include "RTOSQueues.h"
#include "globals.h"
#include "gpio.h"
#include "logger.h"
#include "models.h"

void initSensors(void);

void startSensorsTask(void);

void taskSensors(void *pvParameters);

bool i2cLookup(uint8_t address) ; 
