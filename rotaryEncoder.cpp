
#include "src\rotaryEncoder.h"

// Création du Rotary et de la queue
Rotary encoder(GPIO_ENC_DT, GPIO_ENC_CLK);

//extern AppMode appMode ; 
//QueueHandle_t encoderQueue = nullptr;

// =========================================================
//                     INIT
// =========================================================
void initRotaryEncoder() {
    // Initialise la queue
    if (!encoderQueue) {
        encoderQueue = xQueueCreate(10, sizeof(EncoderEvent_t));
    }

    encoder.begin(GPIO_ENC_DT, GPIO_ENC_CLK);
    //pinMode(GPIO_BOARD_BP, INPUT_PULLUP);
}
// =========================================================
//                  LANCEMENT TASK
// =========================================================
void startRotaryEncoderTask() {
    xTaskCreatePinnedToCore(
        taskRotaryEncoder,
        "Rotary encoder",
        4096,
        NULL,
        PRIORITY_HIGH,      
        NULL,
        CORE_0
    );
}



// =========================================================
//                  ROTARY ENCODER TASK
// =========================================================
void taskRotaryEncoder(void* parameter) {

    bool lastReadState = HIGH;
    bool stableState   = HIGH;
    uint32_t btnPressTime = 0;
    const uint16_t DEBOUNCE_MS = 50;
    uint32_t lastDebounceTime = 0;

    String taskName = pcTaskGetName(NULL) ; 
    logTask(taskName,"▶️ started.");

    int lastPosition = encoder.getPosition();

    while (true) {
        // Mets à jour l'encodeur
        encoder.loop();

        int currentPosition = encoder.getPosition();
        int diff = currentPosition - lastPosition;

        if (abs(diff) > 2) { // 1 cran
            EncoderEvent_t evt = (diff > 0) ? EVT_DOWN : EVT_UP;
            if (encoderQueue) xQueueSend(encoderQueue, &evt, 0);
            lastPosition = currentPosition;
        }

        // Gestion du bouton
        bool reading = digitalRead(GPIO_BOARD_BP);
        if (reading != lastReadState) lastDebounceTime = millis();

        if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
            if (stableState == HIGH && reading == LOW) {
                btnPressTime = millis();
            }

            if (stableState == LOW && reading == HIGH) {
                uint32_t pressDuration = millis() - btnPressTime;
                EncoderEvent_t evt = (pressDuration >= 1000) ? EVT_SELECT_LONG : EVT_SELECT;
                if (encoderQueue) xQueueSend(encoderQueue, &evt, 0);
            }

            stableState = reading;
        }

        lastReadState = reading;

        vTaskDelay(pdMS_TO_TICKS(3));
    }
}

