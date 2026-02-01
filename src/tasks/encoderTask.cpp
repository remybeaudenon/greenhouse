#include "encoderTask.h"
#include "globals.h"

void encoderTask(void *pvParameters) {
    pinMode(GPIO_BOARD_BP, INPUT_PULLUP);
    encoder.begin(GPIO_ENC_DT, GPIO_ENC_CLK);

    bool lastReadState = HIGH;
    bool stableState = HIGH;
    uint32_t btnPressTime = 0;
    const uint16_t DEBOUNCE_MS = 50;
    uint32_t lastDebounceTime = 0;

    while (true) {
        // ----- Rotation -----
        unsigned char r = encoder.process();
        if (r == DIR_CW) {
            EncoderEvent_t evt = EVT_DOWN;
            xQueueSend(encoderQueue, &evt, 0);
        } else if (r == DIR_CCW) {
            EncoderEvent_t evt = EVT_UP;
            xQueueSend(encoderQueue, &evt, 0);
        }

        // ----- Bouton + debounce -----
        bool reading = digitalRead(GPIO_BOARD_BP);

        if (reading != lastReadState) {
            lastDebounceTime = millis();
        }

        if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
            if (stableState == HIGH && reading == LOW) {
                btnPressTime = millis();
            }
            if (stableState == LOW && reading == HIGH) {
                uint32_t pressDuration = millis() - btnPressTime;
                EncoderEvent_t evt =
                    (pressDuration >= 1000) ? EVT_SELECT_LONG : EVT_SELECT;
                xQueueSend(encoderQueue, &evt, 0);
            }
            stableState = reading;
        }

        lastReadState = reading;
        vTaskDelay(pdMS_TO_TICKS(3));
    }
}
