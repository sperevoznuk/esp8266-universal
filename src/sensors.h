#pragma once
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <map>
#include "config.h"
#include "common.h"

namespace sensors
{
    inline float getOffsetByRom(const String &rom) {
        for (const auto &sn : g_config.sensorNames)
            if (sn.rom == rom) return sn.offset;
        return 0.0;
    }

    inline String getNameByRom(const String &rom) {
        for (const auto &sn : g_config.sensorNames)
            if (sn.rom == rom) return sn.name;
        return "";
    }

    struct SensorBus {
        uint8_t pin;
        OneWire *oneWire;
        DallasTemperature *dt;
        int deviceCount = 0; 
    };

    inline std::vector<SensorBus> buses;
    inline std::map<String, float> tempCache;

    inline const unsigned long SENSOR_POLL_MS = 5000UL;
    inline const unsigned long CONVERSION_WAIT_MS = 1200UL;
    inline unsigned long lastRequestAt = 0;
    inline unsigned long conversionStartedAt = 0;
    inline bool conversionPending = false;

    inline void requestConversion() {
        for (auto &b : buses) {
            if (b.deviceCount > 0) b.dt->requestTemperatures();
        }
        conversionPending = true;
        conversionStartedAt = millis();
    }

    inline void updateCache() {
        tempCache.clear();
        for (auto &b : buses) {
            if (b.deviceCount == 0) continue;

            for (int i = 0; i < b.deviceCount; i++) {
                DeviceAddress addr;
                // Читаем адрес и температуру (в асинхронном режиме данные уже готовы)
                if (b.dt->getAddress(addr, i)) {
                    float t = b.dt->getTempC(addr);
                    if (t != DEVICE_DISCONNECTED_C) {
                        char romStr[17];
                        for (int j = 0; j < 8; j++) sprintf(romStr + (j * 2), "%02x", addr[j]);
                        String rom = String(romStr);
                        tempCache[rom] = round2(t + getOffsetByRom(rom));
                    }
                }
            }
        }
        conversionPending = false;
        lastRequestAt = millis();
    }

    inline void tick() {
        unsigned long now = millis();
        if (!conversionPending) {
            if (now - lastRequestAt >= SENSOR_POLL_MS) requestConversion();
        } else {
            if (now - conversionStartedAt >= CONVERSION_WAIT_MS) updateCache();
        }
    }

    inline void begin() {
        buses.clear();
        for (const auto &ch : g_config.channels) {
            if (ch.type == "sensor") {
                bool exists = false;
                for (const auto &b : buses) if (b.pin == ch.pin) exists = true;
                if (exists) continue;

                SensorBus b;
                b.pin = ch.pin;
                b.oneWire = new OneWire(ch.pin);
                b.dt = new DallasTemperature(b.oneWire);
                
                // Инициализируем шину
                b.dt->begin();
                
                // Даем железной подтяжке ESP8266 стабилизировать линию
                delay(100);

                // Ищем датчики (БЕЗ requestTemperatures!)
                int count = b.dt->getDeviceCount();
                logf("Шина DS18B20 на пине %d: найдено устройств = %d", ch.pin, count);
                
                b.deviceCount = count;
                if (count > 0) {
                    b.dt->setWaitForConversion(false);
                }
                
                buses.push_back(b);
            }
        }
        if (!buses.empty()) requestConversion(); 
    }

    inline float getTempByRom(const String &rom) {
        auto it = tempCache.find(rom);
        return (it == tempCache.end()) ? -127.0 : it->second;
    }

    inline void readAll(JsonArray &arr) {
        for (auto &b : buses) {
            if (b.deviceCount == 0) continue;
            for (int i = 0; i < b.deviceCount; i++) {
                DeviceAddress addr;
                if (b.dt->getAddress(addr, i)) {
                    char romStr[17];
                    for (int j = 0; j < 8; j++) sprintf(romStr + (j * 2), "%02x", addr[j]);
                    float temp = getTempByRom(String(romStr));
                    if (temp != -127.0) {
                        JsonObject o = arr.createNestedObject();
                        o["id"] = romStr;
                        o["t"] = fmt2(temp);
                    }
                }
            }
        }
    }

    inline void listAll(JsonArray &arr) {
        for (auto &b : buses) {
            if (b.deviceCount == 0) continue;
            for (int i = 0; i < b.deviceCount; i++) {
                DeviceAddress addr;
                if (b.dt->getAddress(addr, i)) {
                    char romStr[17];
                    for (int j = 0; j < 8; j++) sprintf(romStr + (j * 2), "%02x", addr[j]);
                    JsonObject o = arr.createNestedObject();
                    o["rom"] = romStr;
                    o["t"] = fmt2(getTempByRom(String(romStr)));
                    o["name"] = getNameByRom(String(romStr));
                }
            }
        }
    }
}