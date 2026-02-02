#include "bh1750Task.h"
//#include "globals.h"

// -------- Tâche BH1750 --------
void taskReadBH1750(void *pvParameters) {

  bh1750_data_t data;

  for (;;) {

    data.lux = lightMeter.readLightLevel();

    xQueueOverwrite(queueLux, &data);

    Serial.print("Lux: ");
    Serial.println(data.lux);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

