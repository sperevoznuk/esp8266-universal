#pragma once
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <map>
#include "config.h"
#include "common.h"

namespace sensors
{
    // Получить смещение (offset) по ROM
    inline float getOffsetByRom(const String &rom)
    {
        for (const auto &sn : g_config.sensorNames)
        {
            if (sn.rom == rom)
                return sn.offset;
        }
        return 0.0;
    }

    // Получить имя датчика по ROM (из g_config.sensorNames)
    inline String getNameByRom(const String &rom)
    {
        for (const auto &sn : g_config.sensorNames)
        {
            if (sn.rom == rom)
                return sn.name;
        }
        return "";
    }

    struct SensorBus
    {
        uint8_t pin;
        OneWire *oneWire;
        DallasTemperature *dt;
    };

    inline std::vector<SensorBus> buses;

    // Кэш температуры: ROM -> температура (с учётом offset), -127 = недоступен
    inline std::map<String, float> tempCache;

    // Неблокирующий опрос: конверсия запускается раз в 5 с,
    // результат читается через ~800 мс (12-бит DS18B20 ≈ 750 мс)
    inline const unsigned long SENSOR_POLL_MS = 5000UL;
    inline const unsigned long CONVERSION_WAIT_MS = 1200UL;
    inline unsigned long lastRequestAt = 0;
    inline unsigned long conversionStartedAt = 0;
    inline bool conversionPending = false;

    // Запросить конверсию на всех шинах (возвращается сразу, без блокировки)
    inline void requestConversion()
    {
        for (auto &b : buses)
        {
            b.dt->requestTemperatures();
        }
        conversionPending = true;
        conversionStartedAt = millis();
    }

    // Прочитать результаты конверсии в tempCache
    inline void updateCache()
    {
        tempCache.clear();
        for (auto &b : buses)
        {
            int count = b.dt->getDeviceCount();
            for (int i = 0; i < count; i++)
            {
                DeviceAddress addr;
                if (b.dt->getAddress(addr, i))
                {
                    char romStr[17];
                    for (int j = 0; j < 8; j++)
                    {
                        sprintf(romStr + (j * 2), "%02x", addr[j]);
                    }
                    String rom = String(romStr);
                    float t = b.dt->getTempC(addr);
                    if (t == DEVICE_DISCONNECTED_C)
                        t = -127.0;
                    else
                        t = round2(t + getOffsetByRom(rom));
                    tempCache[rom] = t;
                }
            }
        }
        conversionPending = false;
        lastRequestAt = millis();
    }

    // Неблокирующий «движок»: вызывается каждый проход loop()
    inline void tick()
    {
        unsigned long now = millis();
        if (!conversionPending)
        {
            if (now - lastRequestAt >= SENSOR_POLL_MS)
                requestConversion();
        }
        else
        {
            if (now - conversionStartedAt >= CONVERSION_WAIT_MS)
                updateCache();
        }
    }

    // Инициализация шин датчиков (может быть несколько)
    inline void begin()
    {
        buses.clear();
        for (const auto &ch : g_config.channels)
        {
            if (ch.type == "sensor")
            {
                bool exists = false;
                for (const auto &b : buses)
                    if (b.pin == ch.pin)
                        exists = true;

                if (!exists)
                {
                    SensorBus b;
                    b.pin = ch.pin;
                    b.oneWire = new OneWire(ch.pin);
                    b.dt = new DallasTemperature(b.oneWire);
                    b.dt->begin();
                    b.dt->setWaitForConversion(false); // асинхронная конверсия, без блокировки loop
                    buses.push_back(b);
                    logf("Шина DS18B20 инициализирована на пине %d", ch.pin);
                }
            }
        }
        if (!buses.empty())
            requestConversion(); // первый опрос сразу после старта
    }

    // Получить температуру по ROM-адресу (из кэша, без опроса шины)
    inline float getTempByRom(const String &rom)
    {
        auto it = tempCache.find(rom);
        if (it == tempCache.end())
            return -127.0; // Ещё не отчитался или не найден
        return it->second;
    }

    // Чтение всех датчиков в JSON-массив (из кэша)
    inline void readAll(JsonArray &arr)
    {
        for (auto &b : buses)
        {
            int count = b.dt->getDeviceCount();
            for (int i = 0; i < count; i++)
            {
                DeviceAddress addr;
                if (b.dt->getAddress(addr, i))
                {
                    char romStr[17];
                    for (int j = 0; j < 8; j++)
                    {
                        sprintf(romStr + (j * 2), "%02x", addr[j]);
                    }
                    String rom = String(romStr);
                    float temp = getTempByRom(rom);
                    // Если датчик не ответил, не включаем его в массив
                    if (temp != -127.0)
                    {
                        JsonObject o = arr.createNestedObject();
                        o["id"] = romStr;
                        o["t"] = fmt2(temp);
                    }
                }
            }
        }
    }

    inline void listAll(JsonArray &arr)
    {
        for (auto &b : buses)
        {
            int count = b.dt->getDeviceCount();
            for (int i = 0; i < count; i++)
            {
                DeviceAddress addr;
                if (b.dt->getAddress(addr, i))
                {
                    char romStr[17];
                    for (int j = 0; j < 8; j++)
                    {
                        sprintf(romStr + (j * 2), "%02x", addr[j]);
                    }
                    JsonObject o = arr.createNestedObject();
                    o["rom"] = romStr;
                    float temp = getTempByRom(String(romStr));
                    o["t"] = fmt2(temp);
                    o["name"] = getNameByRom(String(romStr));
                }
            }
        }
    }

}
