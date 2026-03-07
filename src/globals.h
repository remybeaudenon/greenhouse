#include <stdint.h>
#pragma once
#include <Arduino.h>
// ----------------------------------
// Codes et defines
// ----------------------------------
#define MODE_AUTO   'A'
#define MODE_MANU   'M'
#define MODE_OFF    '0'
//#define MODE_FORCE  'F'

// ----------------------------------
// DIGITAL GPIO FORCE BIT REGISTER  
// ----------------------------------
#define FORCE_FREE00_MASK    (1 << 0)
#define FORCE_GPIO35_MASK    (1 << 1)
#define FORCE_GPIO02_MASK    (1 << 2)
#define FORCE_GPIO03_MASK    (1 << 3)
#define FORCE_GPIO04_MASK    (1 << 4)
#define FORCE_GPIO05_MASK    (1 << 5)
#define FORCE_GPIO06_MASK    (1 << 6)

// ----------------------------------
// PWM  GPIO FORCE BIT REGISTER  
// ----------------------------------
#define FORCE_GPIO07_MASK    (1 << 7)


// =========================================================
//                     MODES APPLICATION
// =========================================================
enum AppMode {
  FAN_STOP,
  FAN_ON,
  HEATER_ON,
  HEATER_OFF
};

typedef struct {
    char code;
    const char* label;
} ModeCtxMappings_t;

extern const ModeCtxMappings_t modeCtxMappings[];
extern const uint8_t modeCtxMappingsCount;



const char* getModeCtxLabel(char code);

/*  globals.cpp 
const ModeCtxMappings_t modeCtxMappings[] = {
    {'A', "Auto"},
    {'M', "Manuel"},
    {'0', "Off"},
    {'F', "Force"}
};
*/
typedef struct  {
    char mode;   // 'A' auto, 'M' manuel, 'O' off, 'F'  Force 
} ModeEvent_t ;


// for Queue 
typedef struct  {
    char mode;   // 'A' auto, 'M' manuel, 'O' off, 'F'  Force 
} ModeCtx_t ;





// =========================================================
//   RTOS QUEUES DATA 
// =========================================================
typedef enum {
  EVT_NONE,
  EVT_UP,
  EVT_DOWN,
  EVT_SELECT,
  EVT_SELECT_LONG
} EncoderEvent_t;

// =========================================================
//   Data Models   
// =========================================================

typedef struct {
  int   lux;
  int   humidity;
  float temperature; 
  uint8_t counter ; 
} sensors_dataModel_t;


// =========================================================
//                     MENU
// =========================================================
struct MenuItem {
  const char* label;
  AppMode id;
};
// =========================================================
//                     RTOS TASKS  
// =========================================================
enum CoreNumber {
    CORE_0 = 0,  
    CORE_1 = 1   
    
};
enum TaskPriority {
    PRIORITY_BACKGROUND = 0,   // tâches très lentes, logs
    PRIORITY_LOW        = 1,   // capteurs lents (BH1750, température)
    PRIORITY_MEDIUM     = 2,   // UI, menu, communication LoRa
    PRIORITY_HIGH       = 3,   // encodeur, régulation rapide
    PRIORITY_CRITICAL   = 4    // sécurité, urgence
};

// =========================================================
// =========================================================






