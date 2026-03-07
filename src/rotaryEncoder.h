#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Rotary.h>

#include "globals.h"
#include "RTOSQueues.h"
#include "gpio.h"
#include "logger.h"


// Objet Rotary
extern Rotary encoder;

// Fonctions publiques
void initRotaryEncoder();

void startRotaryEncoderTask();

// Tâche principale de logique automate
void taskRotaryEncoder(void *pvParameters);



