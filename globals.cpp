#include "src\globals.h"

const ModeCtxMappings_t modeCtxMappings[] = {
    {'A', "Mode Auto"},
    {'M', "Mode Manuel"},
    {'0', "Mode Off"},
    {'F', "Mode Force"}
};

const uint8_t modeCtxMappingsCount = sizeof(modeCtxMappings)/sizeof(modeCtxMappings[0]);

const char* getModeCtxLabel(char code) {
    for (uint8_t i = 0; i < modeCtxMappingsCount; i++) {
        if (modeCtxMappings[i].code == code) {
            return modeCtxMappings[i].label;
        }
    }
    return "unknown mode";
}
