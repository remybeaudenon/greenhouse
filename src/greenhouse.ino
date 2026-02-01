#include "globals.h"
#include "tasks/ledTask.h"
#include "tasks/encoderTask.h"
#include "tasks/menuTask.h"
#include "tasks/bh1750Task.h"

TwoWire I2CBH1750(0);        // Bus séparé pour BH1750
BH1750 lightMeter(0x23);     // Adresse BH1750
SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);
Rotary encoder;

QueueHandle_t encoderQueue;
QueueHandle_t queueLux;
LedMode ledMode = LED_TRIM;
volatile uint8_t menuIndex = 3;
MenuItem menuItems[] = {
    { "LED OFF", LED_OFF },
    { "LED ON", LED_ON },
    { "LED BLINK", LED_BLINK },
    { "LED TRIM", LED_TRIM }
};
const uint8_t MENU_COUNT = sizeof(menuItems)/sizeof(menuItems[0]);
bh1750_data_t lastLux;

void setup() {
    Serial.begin(115200);
    initGPIO();

    // I2C BH1750
    I2CBH1750.begin(GPIO_BH1750_SDA, GPIO_BH1750_SCL);
    if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &I2CBH1750))
        Serial.println("BH1750 non détecté !");

    // OLED
    display.init();
    display.clear();
    display.display();

    // Queues
    encoderQueue = xQueueCreate(20, sizeof(EncoderEvent_t));
    queueLux = xQueueCreate(1, sizeof(bh1750_data_t));

    // Lancer les tasks
    xTaskCreate(TaskLED, "TaskLED", 2048, NULL, 1, NULL);
    xTaskCreatePinnedToCore(encoderTask, "EncoderTask", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(menuTask, "MenuTask", 4096, NULL, 2, NULL, 1);
    xTaskCreate(taskReadBH1750, "BH1750Task", 4096, NULL, 1, NULL);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));  // loop vide
}
