#include "src\displayCtrl.h"

//extern SSD1306Wire display ; 

static SSD1306Wire display(
  0x3c,
  500000,
  SDA_OLED,
  SCL_OLED,
  GEOMETRY_128_64,
  RST_OLED
);


// File des événements encodeur
extern QueueHandle_t encoderQueue;
extern QueueHandle_t queueSensorDataModel;

AppMode appMode = HEATER_OFF;

MenuItem menuItems[] = {
  { "FAN STOP",    FAN_STOP },
  { "FAN ON", FAN_ON },
  { "HEATER ON",  HEATER_ON },
  { "HEATER OFF",   HEATER_OFF }
};

const uint8_t MENU_COUNT = sizeof(menuItems)/sizeof(menuItems[0]);

volatile uint8_t menuIndex = 3;

uint8_t getActiveIndex() {
  for (uint8_t i = 0; i < MENU_COUNT; i++) {
    if (menuItems[i].id == appMode)
      return i;
  }
  return 0;
}

void initDisplayCtrl(void) {

  display.init();
  display.clear();
  display.display();

}

void startDisplayCtrlTask(void) {

  xTaskCreatePinnedToCore( taskDisplayCtrl, "Task Display Controler",  4096, NULL, PRIORITY_MEDIUM, NULL, CORE_1);

}

// =========================================================
//               AFFICHAGE OLED
// =========================================================
void drawMenu() {

  sensors_dataModel_t sensors_dataModel , sensors_dataModel_q = {0} ;

  if ( queueSensorDataModel != nullptr  and  xQueuePeek(queueSensorDataModel, &sensors_dataModel_q, 0) == pdTRUE )
   sensors_dataModel = sensors_dataModel_q ; 
  
  display.clear();

  // ----- BARRE STATUS -----
  display.setColor(WHITE);
  display.fillRect(0, 0, 128, 14);

  display.setColor(BLACK);
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  char buf[50];
  sprintf(buf, "%5d Lux | %3.1f° | %2d%% RH", sensors_dataModel.lux, sensors_dataModel.temperature, sensors_dataModel.humidity);
  display.drawString(4, 2, buf);

  // ----- ZONE MENU -----
  display.setColor(WHITE);
  display.drawRect(0, 14, 128, 50);

  const uint8_t visibleItems = 3;
  int start = (menuIndex >= visibleItems)
              ? menuIndex - visibleItems + 1
              : 0;

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

  // Etoile mode actif
  display.setColor(WHITE);

  uint8_t activeIndex = getActiveIndex();

  if (activeIndex >= start &&
      activeIndex < start + visibleItems) {

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

void taskDisplayCtrl(void *pvParameters){

  EncoderEvent_t evt;
  
  //modeCtxMappings
  char MODE_LIST[] = {'A', 'M', '0', 'F'};
  uint32_t lastRefresh = 0;
  
  //ModeEvent_t modeEvent = { MODE_AUTO } ;
  ModeCtx_t modeCtx = {MODE_AUTO} ;    

  String taskName = pcTaskGetName(NULL) ; 
  logTask(taskName,"▶️ started.");

  drawMenu();

  while (true) {

    if (xQueueReceive(encoderQueue,  &evt, pdMS_TO_TICKS(50))) 
    {
      switch (evt) {

        case EVT_UP:
          if (menuIndex > 0) menuIndex--;
          break;

        case EVT_DOWN:
          if (menuIndex < MENU_COUNT - 1)
            menuIndex++;
          break;

        case EVT_SELECT:
        
          appMode = menuItems[menuIndex].id;
          modeCtx.mode  = MODE_LIST[menuIndex] ; 
          xQueueOverwrite(modeCtxQueue, &modeCtx);
          break;

        default:
          break;
      }

      drawMenu();
    }

    // 👉 RAFRAICHISSEMENT DYNAMIQUE
    if (millis() - lastRefresh > 1000) {
      drawMenu();
      lastRefresh = millis();
    }
  }

}