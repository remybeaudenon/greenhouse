#include "bh1750Task.h"
#include "globals.h"

void taskReadBH1750(void *pvParameters) {
    while (true) {
        float lux = lightMeter.readLightLevel();
        lastLux.lux = lux;

        // Mettre dans queue si quelqu'un veut lire en dehors
        xQueueOverwrite(queueLux, &lastLux);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
