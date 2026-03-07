
#include "src\logger.h"
#include <stdarg.h>

void logTask(const String &name, const String &msg) {
    String taskName = pcTaskGetName(NULL) ; 
    Serial.printf("Rtos::Task %s %s\n", taskName.c_str(), msg.c_str());
}

void logTask1(const String &msg) {
    String taskName = pcTaskGetName(NULL) ; 
    Serial.printf("Rtos::Task %s %s\n", taskName.c_str(), msg.c_str());
}

void logfTask(const char* fmt, ...) {
    char buffer[256];  // taille à adapter selon tes besoins
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    logTask1(buffer);
}

