#include "menuTask.h"
#include "globals.h"

void drawMenu() {
    display.clear();
    display.drawRect(0, 0, 128, 64);

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 2, "Polytech Student test");

    // Barre lux
    char buf[16];
    snprintf(buf, sizeof(buf), "Ambient: %.1f lx", lastLux.lux);
    display.drawString(64, 14, buf);

    display.drawLine(0, 16, 128, 16);

    // Menu
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    const uint8_t visibleItems = 3;
    int start = (menuIndex >= visibleItems) ? menuIndex - visibleItems + 1 : 0;

    for (uint8_t i = 0; i < visibleItems; i++) {
        uint8_t idx = start + i;
        if (idx >= MENU_COUNT) break;
        uint8_t y = 18 + i * 14;

        if (idx == menuIndex) {
            display.fillRect(5, y, 118, 12);
            display.setColor(BLACK);
        } else {
            display.setColor(WHITE);
        }

        display.drawString(10, y, menuItems[idx].label);
    }

    // Étoile pour mode actif
    display.setColor(WHITE);
    uint8_t activeIndex = 0;
    for (uint8_t i = 0; i < MENU_COUNT; i++) {
        if (menuItems[i].id == ledMode) activeIndex = i;
    }

    if (activeIndex >= start && activeIndex < start + visibleItems) {
        uint8_t y = 18 + (activeIndex - start) * 14;
        if (activeIndex == menuIndex) {
            display.setColor(BLACK);
            display.drawString(105, y, "*");
            display.setColor(WHITE);
        } else {
            display.drawString(105, y, "*");
        }
    }

    display.display();
}

void menuTask(void *pvParameters) {
    EncoderEvent_t evt;

    drawMenu();

    while (true) {
        if (xQueueReceive(encoderQueue, &evt, portMAX_DELAY)) {
            switch (evt) {
                case EVT_UP:
                    if (menuIndex > 0) menuIndex--;
                    break;
                case EVT_DOWN:
                    if (menuIndex < MENU_COUNT - 1) menuIndex++;
                    break;
                case EVT_SELECT:
                    ledMode = menuItems[menuIndex].id;
                    break;
                case EVT_SELECT_LONG:
                    Serial.print("Appui long sur : ");
                    Serial.println(menuItems[menuIndex].label);
                    break;
                default: break;
            }
            drawMenu();
        } else {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}
