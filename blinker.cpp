//#include <stdint.h>
#include "src\blinker.h"

BlinkerFlag blinkerFlag =
{
    .durationOnMs = 150,
    .durationOffMs = 850,
    .lastToggle = 0
};

bool blinkerState = false ; 

void initBlinker() {
 

}

void startBlinkerTask(){

  xTaskCreate( blinkerTask ,"Blinker", 4096 , NULL, PRIORITY_LOW, NULL );

}


void blinkerTask(void* pvParameters)
{
    logfTask("▶️ started.");

    while(true)
    {
        uint32_t now =  millis();

        uint32_t interval = blinkerState ?   blinkerFlag.durationOnMs :  blinkerFlag.durationOffMs;

        if(interval > 0)
        {
            if(now - blinkerFlag.lastToggle >= interval)
            {
                blinkerState = !blinkerState;
                blinkerFlag.lastToggle = now;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

