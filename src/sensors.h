#pragma once
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include "config.h"
#include "common.h"

namespace sensors {

    struct SensorBus {
        uint8_t pin;
        OneWire* oneWire;
        DallasTemperature* dt;
    };

    inline std::vector<SensorBus> buses;

    // Инициализация шин датчиков (может быть несколько)
    inline void begin() {
        buses.clear();
        for (const auto& ch : g_config.channels) {
            if (ch.type == "sensor") {
                bool exists = false;
                for (const auto& b : buses) if (b.pin == ch.pin) exists = true;
                
                if (!exists) {
                    SensorBus b;
                    b.pin = ch.pin;
                    b.oneWire = new OneWire(ch.pin);
                    b.dt = new DallasTemperature(b.oneWire);
                    b.dt->begin();
                    buses.push_back(b);
                    logf("Шина DS18B20 инициализирована на пине %d", ch.pin);
                }
            }
        }
    }

    // Получить температуру по ROM-адресу
    inline float getTempByRom(const String& rom) {
        for (auto& b : buses) {
            b.dt->requestTemperatures();
            int count = b.dt->getDeviceCount();
            
            for (int i = 0; i < count; i++) {
                DeviceAddress addr;
                if (b.dt->getAddress(addr, i)) {
                    char romStr[17];
                    for (int j = 0; j < 8; j++) {
                        sprintf(romStr + (j * 2), "%02x", addr[j]);
                    }
                    if (rom == String(romStr)) {
                        return b.dt->getTempC(addr);
                    }
                }
            }
        }
        return -127.0; // Не найден
    }

    // Чтение всех датчиков в JSON-массив
    inline void readAll(JsonArray& arr) {
        for (auto& b : buses) {
            b.dt->requestTemperatures();
            int count = b.dt->getDeviceCount();
            
            for (int i = 0; i < count; i++) {
                DeviceAddress addr;
                if (b.dt->getAddress(addr, i)) {
                    float temp = b.dt->getTempC(addr);
                    
                    // Если датчик не ответил, не включаем его в массив
                    if (temp != DEVICE_DISCONNECTED_C) {
                        // Формируем ROM в виде 12-символьной hex-строки
                        char romStr[17];
                        for (int j = 0; j < 8; j++) {
                            sprintf(romStr + (j * 2), "%02x", addr[j]);
                        }
                        
                        JsonObject o = arr.createNestedObject();
                        o["id"] = romStr;
                        o["t"] = temp;
                    }
                }
            }
        }
    }


     inline void listAll(JsonArray& arr) {
        for (auto& b : buses) {
            int count = b.dt->getDeviceCount();
            for (int i = 0; i < count; i++) {
                DeviceAddress addr;
                if (b.dt->getAddress(addr, i)) {
                    char romStr[17];
                    for (int j = 0; j < 8; j++) {
                        sprintf(romStr + (j * 2), "%02x", addr[j]);
                    }
                    JsonObject o = arr.createNestedObject();
                    o["rom"] = romStr;
                    o["t"] = b.dt->getTempC(addr);
                }
            }
        }
    }
}
