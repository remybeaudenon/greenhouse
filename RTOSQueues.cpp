#include "src\RTOSQueues.h"

// Définition réelle des variables
QueueHandle_t encoderQueue            = nullptr;
QueueHandle_t queueSensorDataModel    = nullptr;
QueueHandle_t modeCtxQueue            = nullptr;
QueueHandle_t lokiQueue               = nullptr;

void createQueues() {

  encoderQueue = xQueueCreate( ENCODER_QUEUE_SIZE, sizeof(EncoderEvent_t));
  queueSensorDataModel = xQueueCreate(   SENSORS_DATA_MODEL_QUEUE_SIZE, sizeof(GreenhouseSensorsModel_t));
  modeCtxQueue    = xQueueCreate(  MODE_CTX_QUEUE_SIZE  , sizeof(ModeCtx_t));
  lokiQueue = xQueueCreate(LOKI_MESSAGE_QUEUE_SIZE, sizeof(LokiMessage_t)  );
}
