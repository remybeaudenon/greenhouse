#pragma once
#include <stdint.h>
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "globals.h"
#include "RTOSQueues.h"
#include "gpio.h"
#include "src\logger.h"


extern AppMode appMode ; 


// Fonctions publiques
void initPLC();

void startPLCTask();

// Tâche principale de logique automate
void taskPLC(void *pvParameters);

void applyDigitalOutput(uint32_t bitMask, uint8_t pin, bool normalValue); 

void applyAnalogOutput(uint32_t bitMask, uint8_t pin, int normalValue); 


