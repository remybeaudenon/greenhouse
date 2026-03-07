//#include "esp32-hal.h"
#include <stdint.h>
//#include "esp32-hal-gpio.h"
#include "src\plc.h"
#include "src\models.h"
#include "src\blinker.h"


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

  logfTask("▶️ started  modeCtx.mode: [%c]  label: [%s] ", modeCtx.mode , getModeCtxLabel(modeCtx.mode)  );

  while (true) {

   // -------- INPUTS READING  --------
    if (queueSensorDataModel != nullptr and  xQueuePeek(queueSensorDataModel, &sensors_dataModel_q, 0) == pdTRUE )
      sensors_dataModel = sensors_dataModel_q ;

    if (modeCtxQueue != nullptr and  xQueuePeek(modeCtxQueue, &modeCtx_q, 0) == pdTRUE )
      
      { if (modeCtx.mode != modeCtx_q.mode ) 
        {
          logfTask("modeCtx.mode: [%c]  label: [%s] ", modeCtx_q.mode , getModeCtxLabel(modeCtx_q.mode) );
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
        logfTask("⚠ !!! Defaults mode" ) ;
        vTaskDelay(pdMS_TO_TICKS(5000));
      }
      break; 

    }

    // --- Digital Output ---- 
    bool logicValue = blinkerState  ; 
    applyDigitalOutput(FORCE_GPIO35_MASK, GPIO_BOARD_LED, logicValue);
   
    logicValue = LOW ; 
    applyDigitalOutput(FORCE_GPIO03_MASK, GPIO_FREE_3, logicValue);

    logicValue = LOW ; 
    applyDigitalOutput(FORCE_GPIO04_MASK, GPIO_FREE_4, logicValue);
    
    logicValue = LOW ; 
    applyDigitalOutput(FORCE_GPIO05_MASK, GPIO_FREE_5, logicValue);
    
    logicValue = LOW ; 
    applyDigitalOutput(FORCE_GPIO06_MASK, GPIO_FAN, logicValue);

    // --- DigitalPWM  Output ---- 
    int logicIntValue = 111  ; 
    applyAnalogOutput(FORCE_GPIO07_MASK, GPIO_PWM_HEATER , logicIntValue);

    // FORCE action is in progress Warning LED  !!! 
    logicValue =  (cmdModel.digital_force_enable != 0  ) || (cmdModel.pwm_force_enable != 0  )   ; 
    digitalWrite(GPIO_FORCE_LED, logicValue  &&  ! blinkerState   );

    vTaskDelay(pdMS_TO_TICKS(50));
  }

}

void applyDigitalOutput(uint32_t bitMask,uint8_t pin,bool normalValue)
{
    bool isForced    = (cmdModel.digital_force_enable & bitMask) != 0;
    bool forcedValue = (cmdModel.digital_force_value  & bitMask) != 0;

    bool outputValue = isForced ? forcedValue : normalValue;

    digitalWrite(pin, outputValue);
}

void applyAnalogOutput(uint32_t bitMask,uint8_t pin, int normalValue)
{
    bool isForced    = (cmdModel.pwm_force_enable & bitMask) != 0;
    int forcedValue = cmdModel.pwm_force_value[pin] ;  

    int  outputValue = isForced ? forcedValue : normalValue;

    analogWrite(pin, outputValue);
}




