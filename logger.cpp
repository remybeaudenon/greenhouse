
#include "src\logger.h"

void logTask(const String &name, const String &msg) {
    Serial.printf("Rtos::Task %s %s\n", name.c_str(), msg.c_str());
}

