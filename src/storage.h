#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h> // Используем ArduinoJson v6
#include "config.h"
#include "common.h"

namespace storage {

    const char* CONFIG_FILE = "/config.json";

    // Сохранение конфига
    inline bool save() {
        DynamicJsonDocument doc(4096);
        
        doc["deviceName"] = g_config.deviceName;
        doc["serverUrl"] = g_config.serverUrl;
        doc["period"] = g_config.period;
        
        JsonArray channelsArray = doc.createNestedArray("channels");
        for (const auto& ch : g_config.channels) {
            JsonObject c = channelsArray.createNestedObject();
            c["type"] = ch.type;
            c["pin"] = ch.pin;
            c["name"] = ch.name;
        }

        JsonArray snArr = doc.createNestedArray("sensorNames");
        for (const auto& sn : g_config.sensorNames) {
            JsonObject o = snArr.createNestedObject();
            o["rom"] = sn.rom;
            o["name"] = sn.name;
        }

        JsonArray rulesArray = doc.createNestedArray("rules");
        for (const auto& r : g_config.rules) {
            JsonObject jr = rulesArray.createNestedObject();
            jr["id"] = r.id;
            jr["enabled"] = r.enabled;
            jr["name"] = r.name;
            jr["target"] = r.target;
            jr["hysteresis"] = r.hysteresis;
            
            JsonObject cond = jr.createNestedObject("cond");
            cond["type"] = r.cond.type;
            if (r.cond.type == "sensor") cond["rom"] = r.cond.rom;
            else cond["pin"] = r.cond.pin;
            
            JsonArray acts = jr.createNestedArray("actuators");
            for (uint8_t pin : r.actuators) acts.add(pin);
        }

        File file = LittleFS.open(CONFIG_FILE, "w");
        if (!file) {
            logf("Не удалось открыть файл для записи");
            return false;
        }
        
        serializeJson(doc, file);
        file.close();
        
        logf("Конфигурация сохранена в LittleFS");
        return true;
    }

    // Загрузка конфига
    inline bool load() {
        if (!LittleFS.exists(CONFIG_FILE)) {
            logf("Файл конфигурации не найден.");
            return false;
        }

        File file = LittleFS.open(CONFIG_FILE, "r");
        if (!file) {
            logf("Не удалось открыть файл для чтения");
            return false;
        }

        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error) {
            logf("Ошибка парсинга JSON: %s", error.c_str());
            return false;
        }

        g_config.deviceName = doc["deviceName"] | "";
        g_config.serverUrl = doc["serverUrl"] | "";
        g_config.period = doc["period"] | 60;
        
        g_config.channels.clear();
        for (JsonObject c : doc["channels"].as<JsonArray>()) {
            Channel ch;
            ch.type = c["type"].as<String>();
            ch.pin = c["pin"].as<uint8_t>();
            ch.name = c["name"] | "";
            g_config.channels.push_back(ch);
        }

        g_config.rules.clear();
        for (JsonObject jr : doc["rules"].as<JsonArray>()) {
            Rule r;
            r.id = jr["id"].as<String>();
            r.enabled = jr["enabled"] | true;
            r.name = jr["name"] | "";
            r.target = jr["target"] | 0.0;
            r.hysteresis = jr["hysteresis"] | 0.5;
            
            r.cond.type = jr["cond"]["type"].as<String>();
            if (r.cond.type == "sensor") r.cond.rom = jr["cond"]["rom"].as<String>();
            else r.cond.pin = jr["cond"]["pin"].as<uint8_t>();
            
            for (JsonVariant v : jr["actuators"].as<JsonArray>()) {
                r.actuators.push_back(v.as<uint8_t>());
            }
            g_config.rules.push_back(r);
        }

        g_config.sensorNames.clear();
        for (JsonObject sn : doc["sensorNames"].as<JsonArray>()) {
            SensorNameEntry e;
            e.rom = sn["rom"].as<String>();
            e.name = sn["name"] | "";
            g_config.sensorNames.push_back(e);
        }

        logf("Конфигурация загружена. Каналов: %d, Правил: %d, Имён датчиков: %d", g_config.channels.size(), g_config.rules.size(), g_config.sensorNames.size());
        return true;
    }

    inline void begin() {
        if (!LittleFS.begin()) {
            logf("LittleFS mount FAILED");
        } else {
            logf("LittleFS OK");
            load();
        }
    }
}
