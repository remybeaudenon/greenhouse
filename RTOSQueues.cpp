#include "src\RTOSQueues.h"

// Définition réelle des variables
QueueHandle_t encoderQueue            = nullptr;
QueueHandle_t queueSensorDataModel    = nullptr;
QueueHandle_t modeCtxQueue        = nullptr;

void createQueues() {

  encoderQueue = xQueueCreate( 20, sizeof(EncoderEvent_t));
  queueSensorDataModel = xQueueCreate(      1, sizeof(GreenhouseSensorsModel_t));
  modeCtxQueue    = xQueueCreate(     1, sizeof(ModeCtx_t));
}
