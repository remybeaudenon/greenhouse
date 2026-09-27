#pragma once
#include <Arduino.h>
#include "globals.h"
//#include "RTOSQueues.h"

void logTask(const String &name , const String &msg) ;
void logfTask(LogLevel_t level, const char* fmt, ...);
void logfTask(const char* fmt, ...) ; 

// Initialisation de la queue de logs
//void loggerInit();

