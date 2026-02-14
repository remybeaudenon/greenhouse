#include "src\plc.h"

// On réutilise la dernière valeur connue
//extern bh1750_data_t lastLux;
//bh1750_data_t lastLux;

/*
const char* AppModeNames[] = {
    "FAN STOP",
    "FAN ON",
    "HEATER ON",
    "HEATER OFF",
    "----------"
};
*/


void initPLC() {

}

void startPLCTask(){
  xTaskCreate( taskPLC,"PLC Logic", 4096, NULL, PRIORITY_LOW,   NULL );
}


void taskPLC(void *pvParameters) {

  ModeCtx_t modeCtx_q, modeCtx  = { .mode = '0' } ; 

  sensors_dataModel_t sensors_dataModel , sensors_dataModel_q  = {0}  ; 

  String taskName = pcTaskGetName(NULL) ; 
  logTask(taskName,"▶️ started.");

  char buf[128];  // enought size
  snprintf(buf, sizeof(buf), "modeCtx.mode: [%c]  label: [%s] ", modeCtx.mode , getModeCtxLabel(modeCtx.mode)  );
  logTask(taskName, buf ) ;

  while (true) {

   // -------- INPUTS READING  --------
    if (queueSensorDataModel != nullptr and  xQueuePeek(queueSensorDataModel, &sensors_dataModel_q, 0) == pdTRUE )
      sensors_dataModel = sensors_dataModel_q ;

    if (modeCtxQueue != nullptr and  xQueuePeek(modeCtxQueue, &modeCtx_q, 0) == pdTRUE )
      
      { if (modeCtx.mode != modeCtx_q.mode ) 
        {
          snprintf(buf, sizeof(buf), "modeCtx.mode: [%c]  label: [%s] ", modeCtx_q.mode , getModeCtxLabel(modeCtx_q.mode) );
          logTask(taskName, buf ) ;
          modeCtx = modeCtx_q ; 
        }
      }  
   // -------- INPUTS READING  --------

   // -------- LOGIQUE ----------------
    switch (modeCtx.mode) {
      case 'M' :    //case FAN_ON:
        digitalWrite(GPIO_FAN, HIGH);
        //digitalWrite(GPIO_BOARD_LED, HIGH);
        break;

      case 'A' :   //case FAN_STOP:
        digitalWrite(GPIO_FAN, LOW);
        //digitalWrite(GPIO_BOARD_LED, LOW);
        break;

      case 'F' :   //case HEATER_OFF:
        analogWrite(GPIO_PWM_HEATER, 0);
        break;

      case '0' :   //case HEATER_ON:
      {

        int lux = sensors_dataModel.lux;

        // Saturation bornes
        if (lux <= 0)
          lux = 0;

        if (lux >= 1000)
          lux = 255;

        // Mise à l'échelle 0–1000 lx → 0–255 PWM
        uint8_t power =
          static_cast<uint8_t>((lux * 255.0f) / 1000.0f);

        analogWrite(GPIO_PWM_HEATER, power);
      }
      break;

      default: 
      {
        logTask(taskName, " !!! Defaults mode" ) ;
        vTaskDelay(pdMS_TO_TICKS(5000));
      }
      break; 

    }


    digitalWrite(GPIO_BOARD_LED,(modeCtx.mode == 'M' || modeCtx.mode == '0' ) ) ;   


    vTaskDelay(pdMS_TO_TICKS(50));
  }

}