#pragma once
#include <Arduino.h>
#include "globals.h"
#include "src\logger.h"

struct BlinkerFlag {
    uint32_t durationOnMs;  // durée allumée
    uint32_t durationOffMs; // durée éteinte
    uint32_t lastToggle;    // timestamp dernier basculement
};

//BlinkerFlag blinkerFlag = { 250, 750 , false, 0}; // par défaut 

extern  bool blinkerState;    

void initBlinker();

void startBlinkerTask() ; 

void blinkerTask(void *pvParameters);



