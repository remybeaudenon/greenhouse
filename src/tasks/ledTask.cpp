#include "ledTask.h"
#include "globals.h"

void TaskLED(void *pvParameters) {
    pinMode(GPIO_BOARD_LED, OUTPUT);

    while (true) {
        switch (ledMode) {
            case LED_ON:
                analogWrite(GPIO_BOARD_LED, 255);
                vTaskDelay(pdMS_TO_TICKS(100));
                break;

            case LED_OFF:
                analogWrite(GPIO_BOARD_LED, 0);
                vTaskDelay(pdMS_TO_TICKS(100));
                break;

            case LED_BLINK:
                analogWrite(GPIO_BOARD_LED, 255);
                vTaskDelay(pdMS_TO_TICKS(500));
                analogWrite(GPIO_BOARD_LED, 0);
                vTaskDelay(pdMS_TO_TICKS(500));
                break;

            case LED_TRIM:
                for (int duty = 0; duty <= 255 && ledMode == LED_TRIM; duty++) {
                    analogWrite(GPIO_BOARD_LED, duty);
                    vTaskDelay(pdMS_TO_TICKS(5));
                }
                vTaskDelay(pdMS_TO_TICKS(500));
                for (int duty = 255; duty >= 0 && ledMode == LED_TRIM; duty--) {
                    analogWrite(GPIO_BOARD_LED, duty);
                    vTaskDelay(pdMS_TO_TICKS(5));
                }
                vTaskDelay(pdMS_TO_TICKS(500));
                break;
        }
    }
}
