#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Rotary.h>

#include "globals.h"
#include "RTOSQueues.h"
#include "gpio.h"
#include "logger.h"

// GPIO pins pour l'encodeur
//#define GPIO_ENC_DT  25  // À ajuster selon ton câblage
//#define GPIO_ENC_CLK 26
//#define GPIO_BOARD_BP 27  // Bouton de l'encodeur

/*
// Queue pour les événements
extern QueueHandle_t encoderQueue;
*/
// Objet Rotary
extern Rotary encoder;

// Fonctions publiques
void initRotaryEncoder();

void startRotaryEncoderTask();

// Tâche principale de logique automate
void taskRotaryEncoder(void *pvParameters);



