#pragma once

#ifndef RTOSQUEUES_H
#define RTOSQUEUES_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "globals.h"
#include "models.h"

#define ENCODER_QUEUE_SIZE                1
#define SENSORS_DATA_MODEL_QUEUE_SIZE     1
#define MODE_CTX_QUEUE_SIZE               1
#define LOKI_MESSAGE_QUEUE_SIZE           50

// =========================================================
//              FILES DE COMMUNICATION RTOS
// =========================================================

// File des événements encodeur
extern QueueHandle_t encoderQueue;

// Sensor dataModel Queue
extern QueueHandle_t queueSensorDataModel;
extern QueueHandle_t modeCtxQueue;  

// Loki logger Queue
extern QueueHandle_t lokiQueue;  

// =========================================================
//              FONCTIONS UTILITAIRES
// =========================================================

// Création centralisée des queues
void createQueues();


// =========================================================
#endif // RTOSQUEUES_H
// =========================================================




