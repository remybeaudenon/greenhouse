#pragma once
#include <stdint.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>


#include "logger.h"


extern Preferences prefs;


// # --- MODELS --- 
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
        StaticJsonDocument<750> doc;
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
        prefs.begin("config", false);

        prefs.putString("cmdModel_v1",  toJson() );
        prefs.end();

    }

    void load() {
        prefs.begin("config", false);
        if (!prefs.isKey("cmdModel_v1")) {
            fromJson("{}");
            prefs.putString("cmdModel_v1", toJson() );
        }
        String aString  = prefs.getString("cmdModel_v1" ) ; 
        fromJson(aString) ; 
        prefs.end();
    }

};

extern GreenhouseCmdModel_t cmdModel;


// # --- MODELS --- 
struct GreenhouseSensorsModel_t {
    int temperature;  // Stocké en 1/10e (ex: 204 pour 20.4°C)
    int humidity;
    int light;
    bool modelDirty;

    // --- Vers le JSON (pour envoi ou debug) ---
    String toJson() const {
        StaticJsonDocument<128> doc; // 128 octets suffisent largement ici
        doc["temp"] = temperature * 0.1f;
        doc["hum"]  = humidity;
        doc["lux"]  = light;

        String out;
        serializeJson(doc, out);
        return out;
    }

    // --- Du JSON vers la Structure (utile pour du monitoring ou simulateur) ---
    void fromJson(const String& json) {
        StaticJsonDocument<750> doc;
        deserializeJson(doc, json);
        
        // On reconvertit le float en int (1/10e) proprement
        if (doc.containsKey("temp")) {
            temperature = (int)(doc["temp"].as<float>() * 10.0f + 0.5f);
        }
        humidity = doc["hum"] | 0;
        light    = doc["lux"] | 0;
        modelDirty = true; 
    }
    void save() {
        prefs.begin("config", false);
        prefs.putString("cmdModel",  toJson() );
        prefs.end();

    }

    void load() {
        prefs.begin("config", false);
        if (!prefs.isKey("sensorsModel")) {
            prefs.putString("sensorsModel", toJson() );
        }
        String aString  = prefs.getString("sensorModel" ) ; 
        fromJson(aString) ; 
        prefs.end();
    }

};

extern GreenhouseSensorsModel_t sensorsModel;







