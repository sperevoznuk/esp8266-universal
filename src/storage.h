#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "config.h"
#include "common.h"
#include "config_json.h"

namespace storage {

    const char* CONFIG_FILE = "/config.json";

    inline bool save() {
        DynamicJsonDocument doc(4096);
        g_config.configVersion++;
        configJson::toJson(g_config, doc, /*includeWifi=*/false);

        File file = LittleFS.open(CONFIG_FILE, "w");
        if (!file) {
            logf("Не удалось открыть файл для записи");
            return false;
        }
        serializeJson(doc, file);
        file.close();
        logf("Конфигурация сохранена в LittleFS (v%u)", g_config.configVersion);
        return true;
    }

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

        if (!configJson::fromJson(doc, g_config)) {
            logf("Конфиг не соответствует схеме");
            return false;
        }

        logf("Конфигурация загружена. Каналов: %d, Правил: %d, Имён датчиков: %d",
             g_config.channels.size(), g_config.rules.size(), g_config.sensorNames.size());
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