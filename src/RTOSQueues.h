#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "globals.h"

// =========================================================
//              FILES DE COMMUNICATION RTOS
// =========================================================

// File des événements encodeur
extern QueueHandle_t encoderQueue;

// Sensor dataModel Queue
extern QueueHandle_t queueSensorDataModel;

extern QueueHandle_t modeCtxQueue;  

// =========================================================
//              FONCTIONS UTILITAIRES
// =========================================================

// Création centralisée des queues
void createQueues();


