
#include <stdarg.h>
#include "src\logger.h"
#include "src\RTOSQueues.h"

/*
enum LogLevel_t {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_DEBUG
};
*/ 
const char* logLevelToString(LogLevel_t level) {

    switch (level) {
        case LOG_INFO:
            return "INFO";
        case LOG_WARNING:
            return "WARNING";
        case LOG_ERROR:
            return "ERROR";
        case LOG_DEBUG:
            return "DEBUG";
        default:
            return "UNKNOWN";
    }
}

void logOldTask(const String &name, const String &msg) {
    String taskName = pcTaskGetName(NULL) ; 
    Serial.printf("Rtos::Task %s %s\n", taskName.c_str(), msg.c_str());
}

void serialLog(const String &msg) {
    String taskName = pcTaskGetName(NULL) ; 
    Serial.printf("Rtos::Task %s %s\n", taskName.c_str(), msg.c_str());

}

void logfTask(LogLevel_t level, const char* fmt, ...) {
    char buffer[256];  
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    serialLog(buffer);

    
    // Add to Log Queue
    if (lokiQueue == nullptr) return;

    LokiMessage_t record = {};
    snprintf(record.level, sizeof(record.level), "%s", logLevelToString(level));
    snprintf(record.taskName, sizeof(record.taskName), "%s",pcTaskGetName(NULL));
    snprintf(record.message, sizeof(record.message), "%s", buffer);

    xQueueSend(lokiQueue, &record, 0);
     
}
