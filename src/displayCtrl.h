#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "HT_SSD1306Wire.h"

#include "globals.h"
#include "RTOSQueues.h"

#include "gpio.h"
#include "logger.h"

void initDisplayCtrl(void) ;

void startDisplayCtrlTask(void);

void taskDisplayCtrl(void *pvParameters);
