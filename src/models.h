#pragma once
#include <stdint.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#include "logger.h"

extern Preferences prefs;

constexpr size_t JSON_DOC_SIZE = 750;
constexpr const char PREFS_NAMESPACE[] = "config";

inline void saveToPrefs(const char* key, const String& json) {
    prefs.begin(PREFS_NAMESPACE, false);
    prefs.putString(key, json);
    prefs.end();
}

inline String loadFromPrefs(const char* key, const String& defaultJson = "{}") {
    prefs.begin(PREFS_NAMESPACE, false);
    if (!prefs.isKey(key)) {
        prefs.putString(key, defaultJson);

    }
    String value = prefs.getString(key);
    prefs.end();
    return value;
}


// ------------

struct MQTTConfig_t {
    char wifi_ssid[32];
    char wifi_password[64];
    char server[64] ; // "PI5-IOT-EDGE-U25.local" ; // 192.168.8.128";
    int  port; //   1883;

    String toJson() const {
        StaticJsonDocument<JSON_DOC_SIZE> doc;
        doc["wifi_ssid"] = wifi_ssid;
        doc["wifi_password"] = wifi_password;
        doc["server"] = server;
        doc["port"] = port;
        String out;
        serializeJson(doc, out);
        return out;
    }
    bool fromJson(const String& json) {
        StaticJsonDocument<JSON_DOC_SIZE> doc;
        if (deserializeJson(doc, json)) return false;
        const char* ssidTmp = doc["wifi_ssid"] | "SSID_REMY";
        strncpy(wifi_ssid, ssidTmp, sizeof(wifi_ssid));
        wifi_ssid[sizeof(wifi_ssid) - 1] = '\0';  // sécurité

        const char* passTmp = doc["wifi_password"] | "Beaud3non.";
        strncpy(wifi_password, passTmp, sizeof(wifi_password));
        wifi_password[sizeof(wifi_password) - 1] = '\0';

        const char* serverTmp = doc["server"] | "PI5-IOT-EDGE-U25.local";
        strncpy(server, serverTmp, sizeof(server));
        server[sizeof(server) - 1] = '\0';

        port = doc["port"] | 1883 ;
        return true;
    }
    void save() {
        saveToPrefs("NX510v", toJson());
    }

    void load() {
        
        //fromJson(loadFromPrefs("CUDY-550C", R"({"wifi_ssid":"CUDY-550C","wifi_password":"0477060671","server":"PI5-IOT-EDGE-U25.local","port":1883})" )) ; 
        fromJson(loadFromPrefs("NX510v", R"({"wifi_ssid":"SSID_REMY","wifi_password":"Beaud3non.","server":"PI5-IOT-EDGE-U25.local","port":1883})" )) ; 

        logfTask("⚙️  load mqttConfig Properties  NX510v:  %s ", toJson().c_str() );
    }

};
extern MQTTConfig_t mqttConfig;


// # --- CMD MODELS 2.0.0 --- 
struct GreenhouseCmdModel_t {
    int modeCtx;
    int rssi;
    int samplingSensors;
    int temperatureMaxi;
    int temperatureMini;
    int hygrometryMaxi;
    int hygrometryMini;
    int ambientLightMaxi;
    int ambientLightMini;
    uint8_t digital_force_enable;
    uint8_t digital_force_value;
    uint8_t pwm_force_enable;
    uint8_t pwm_force_value[8];
    bool modelDirty ; 

    String toJson() const {
        StaticJsonDocument<750> doc;
        doc["mode"] = modeCtx;
        doc["rssi"] = rssi;
        // 
        doc["samp"] = samplingSensors;
        doc["t_max"] = temperatureMaxi * 0.1f;
        doc["t_min"] = temperatureMini * 0.1f;
        doc["h_max"] = hygrometryMaxi;
        doc["h_min"] = hygrometryMini;
        doc["l_max"] = ambientLightMaxi;
        doc["l_min"] = ambientLightMini;
        doc["d_en"]  = digital_force_enable;
        doc["d_val"] = digital_force_value;
        doc["p_en"]  = pwm_force_enable;
        JsonArray pwm = doc.createNestedArray("pwm");
        for (int i = 0; i < 8; i++) pwm.add(pwm_force_value[i]);

        String out;
        serializeJson(doc, out);
        return out;
    }

    bool fromJson(const String& json) {
        StaticJsonDocument<JSON_DOC_SIZE> doc;
        if (deserializeJson(doc, json)) return false;
        modeCtx = doc["mode"] | 0;
        rssi = doc["rssi"] | 0;
        samplingSensors = doc["samp"] | 10;
        temperatureMaxi = (int)((doc["t_max"] | 30.0f) * 10.0f + 0.5f);
        temperatureMini = (int)((doc["t_min"] | 15.0f) * 10.0f + 0.5f);
        hygrometryMaxi = doc["h_max"] | 80;
        hygrometryMini = doc["h_min"] | 20;
        ambientLightMaxi = doc["l_max"] | 1000;
        ambientLightMini = doc["l_min"] | 0;
        digital_force_enable = doc["d_en"] | 0;
        digital_force_value = doc["d_val"] | 0;
        pwm_force_enable = doc["p_en"] | 0;
        JsonArray pwm = doc["pwm"];
        for (int i = 0; i < 8; i++) pwm_force_value[i] = pwm[i] | 0;
        return true;
    }

    void save() {
        saveToPrefs("cmdModel_v1", toJson());
    }

    void load() {
        fromJson(loadFromPrefs("cmdModel_v1"));
        logfTask("⚙️  load [cmdModel] Properties:  %s ", toJson().c_str() );
    }

};
extern GreenhouseCmdModel_t cmdModel;


// # --- SENSORS MODELS 2.0.0 --- 
struct GreenhouseSensorsModel_t {
    float sht31_temperature;
    int   sht31_humidity; 
    int   bh1750_light; 
    float bme280_temperature;
    int   bme280_humidity; 
    int   bme280_pressure;  

    bool modelDirty;

    // --- Vers le JSON (pour envoi ou debug) ---
    String toJson() const {
        StaticJsonDocument<JSON_DOC_SIZE> doc; 

        doc["sht31_temperature"] = sht31_temperature;
        doc["sht31_humidity"] = sht31_humidity; 
        doc["bh1750_light"] = bh1750_light; 
        doc["bme280_temperature"] = bme280_temperature;
        doc["bme280_humidity"] = bme280_humidity; 
        doc["bme280_pressure"] = bme280_pressure;  
        
        String out;
        serializeJson(doc, out);
        return out;
    }

    // --- Du JSON vers la Structure (utile pour du monitoring ou simulateur) ---
    void fromJson(const String& json) {
        StaticJsonDocument<750> doc;
        deserializeJson(doc, json);

        sht31_temperature = doc["sht31_temperature"] | 0.0f;
        sht31_humidity = doc["sht31_humidity"] | 0; 
        bh1750_light = doc["bh1750_light"] | 0; 
        bme280_temperature = doc["bme280_temperature"] | 0.0f;
        bme280_humidity = doc["bme280_humidity"] | 0; 
        bme280_pressure = doc["bme280_pressure"] | 0;  

        modelDirty = true; 
    }

    void save() {
        saveToPrefs("sensorsModel", toJson());
        //logfTask("⚙️  save [sensorsModel] Properties:  %s ", toJson().c_str() );

    }

    void load() {
        fromJson(loadFromPrefs("sensorsModel"));
        logfTask("⚙️  load [sensorsModel] Properties:  %s ", toJson().c_str() );
    }

};
extern GreenhouseSensorsModel_t sensorsModel;
