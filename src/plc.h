#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "globals.h"
#include "RTOSQueues.h"
#include "gpio.h"
#include "logger.h"


extern AppMode appMode ; 
// Fonctions publiques
void initPLC();

void startPLCTask();

// Tâche principale de logique automate
void taskPLC(void *pvParameters);
