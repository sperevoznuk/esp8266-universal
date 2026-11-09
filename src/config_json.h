#pragma once
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include <functional>
#include "config.h"
#include "common.h"

namespace configJson {

    // --- Сериализация ---
    // Единый формат и для LittleFS, и для GET /api/config
    inline void toJson(const Config& cfg, JsonDocument& doc, bool includeWifi) {
        doc["deviceName"] = cfg.deviceName;
        doc["serverUrl"]  = cfg.serverUrl;
        doc["period"]     = cfg.period;
        doc["configVersion"] = cfg.configVersion;

        if (includeWifi) {
            struct station_config conf;
            wifi_station_get_config(&conf);
            doc["ssid"] = String((char*)conf.ssid);
            doc["pass"] = String((char*)conf.password);
        }

        JsonArray chArr = doc.createNestedArray("channels");
        for (const auto& ch : cfg.channels) {
            JsonObject c = chArr.createNestedObject();
            c["type"] = ch.type;
            c["pin"]  = ch.pin;
            c["name"] = ch.name;
        }

        JsonArray rArr = doc.createNestedArray("rules");
        for (const auto& r : cfg.rules) {
            JsonObject jr = rArr.createNestedObject();
            jr["id"]       = r.id;
            jr["enabled"]  = r.enabled;
            jr["name"]     = r.name;
            jr["condType"] = r.condType;
            jr["combine"]  = r.combine;

            if (r.condType == "sensor") {
                JsonArray sensArr = jr.createNestedArray("sensors");
                for (const auto& sc : r.sensors) {
                    JsonObject s = sensArr.createNestedObject();
                    s["rom"]        = sc.rom;
                    s["target"]     = sc.target;
                    s["hysteresis"] = sc.hysteresis;
                    s["mode"]       = sc.mode;
                }
            } else {
                JsonArray pArr = jr.createNestedArray("sourcePins");
                for (uint8_t pin : r.sourcePins) pArr.add(pin);
            }

            JsonArray acts = jr.createNestedArray("actuators");
            for (uint8_t pin : r.actuators) acts.add(pin);
        }

        JsonArray snArr = doc.createNestedArray("sensorNames");
        for (const auto& sn : cfg.sensorNames) {
            JsonObject o = snArr.createNestedObject();
            o["rom"]    = sn.rom;
            o["name"]   = sn.name;
            o["offset"] = sn.offset;
        }
    }

    // --- Десериализация ---
    // Строгий парсинг только нового формата. Возвращает false при невалидном JSON.
    inline bool fromJson(JsonDocument& doc, Config& cfg) {
        if (!doc.is<JsonObject>()) return false;

        cfg.deviceName = doc["deviceName"] | "";
        cfg.serverUrl  = doc["serverUrl"]  | "";
        cfg.period     = doc["period"]     | 0;
        cfg.configVersion = doc["configVersion"] | 0;

        cfg.channels.clear();
        for (JsonObjectConst c : doc["channels"].as<JsonArrayConst>()) {
            Channel ch;
            ch.type = c["type"].as<String>();
            ch.pin  = c["pin"].as<uint8_t>();
            ch.name = c["name"] | "";
            cfg.channels.push_back(ch);
        }

        cfg.rules.clear();
        for (JsonObjectConst jr : doc["rules"].as<JsonArrayConst>()) {
            Rule r;
            r.id       = jr["id"].as<String>();
            r.enabled  = jr["enabled"] | true;
            r.name     = jr["name"]    | "";
            r.condType = jr["condType"].as<String>();
            r.combine  = jr["combine"] | "and";

            if (r.condType == "sensor") {
                for (JsonObjectConst s : jr["sensors"].as<JsonArrayConst>()) {
                    SensorCondition sc;
                    sc.rom        = s["rom"].as<String>();
                    sc.target     = s["target"]     | 0.0;
                    sc.hysteresis = s["hysteresis"] | 0.5;
                    sc.mode       = s["mode"]       | true;
                    sc.active     = false;
                    r.sensors.push_back(sc);
                }
            } else {
                for (JsonVariantConst v : jr["sourcePins"].as<JsonArrayConst>()) {
                    r.sourcePins.push_back(v.as<uint8_t>());
                }
            }

            for (JsonVariantConst v : jr["actuators"].as<JsonArrayConst>()) {
                r.actuators.push_back(v.as<uint8_t>());
            }

            r.prevEnabled = r.enabled;
            cfg.rules.push_back(r);
        }

        cfg.sensorNames.clear();
        for (JsonObjectConst sn : doc["sensorNames"].as<JsonArrayConst>()) {
            SensorNameEntry e;
            e.rom    = sn["rom"].as<String>();
            e.name   = sn["name"]   | "";
            e.offset = sn["offset"] | 0.0;
            cfg.sensorNames.push_back(e);
        }

        return true;
    }

    // --- Валидация ---
    // Проверка уникальности пинов каналов + отсутствие циклов в relay-правилах.
    // Возвращает пустую строку при успехе, иначе — текст ошибки.
    inline String validate(const Config& cfg) {
        // 1. Уникальность пинов каналов
        std::map<uint8_t, int> pinCount;
        for (const auto& ch : cfg.channels) {
            if (++pinCount[ch.pin] > 1) {
                return "Пин " + String(ch.pin) + " используется более одного раза в каналах";
            }
        }

        // 2. Проверка ссылок в правилах и поиск циклов
        std::map<uint8_t, std::vector<uint8_t>> graph; // source_pin -> [actuator_pins]

        for (const auto& r : cfg.rules) {
            if (r.condType == "sensor") {
                if (r.sensors.empty() || r.sensors.size() > 3)
                    return "Правило '" + r.name + "': датчиков должно быть от 1 до 3";
            } else if (r.condType == "button" || r.condType == "relay") {
                if (r.sourcePins.empty() || r.sourcePins.size() > 3)
                    return "Правило '" + r.name + "': источников должно быть от 1 до 3";
            } else {
                return "Правило '" + r.name + "': неизвестный тип условия '" + r.condType + "'";
            }

            if (r.actuators.empty()) {
                return "Правило '" + r.name + "': не выбрано ни одного реле";
            }

            if (r.condType == "relay") {
                for (uint8_t src : r.sourcePins) {
                    // проверяем, что src — реально реле
                    bool srcIsRelay = false;
                    for (const auto& ch : cfg.channels)
                        if (ch.pin == src && ch.type == "relay") srcIsRelay = true;
                    if (!srcIsRelay)
                        return "Правило '" + r.name + "': источник " + String(src) + " не является реле";

                    for (uint8_t dst : r.actuators) {
                        bool dstIsRelay = false;
                        for (const auto& ch : cfg.channels)
                            if (ch.pin == dst && ch.type == "relay") dstIsRelay = true;
                        if (!dstIsRelay)
                            return "Правило '" + r.name + "': актуатор " + String(dst) + " не является реле";
                        graph[src].push_back(dst);
                    }
                }
            }
        }

        // Поиск циклов (DFS)
        std::map<uint8_t, int> visited; // 0=не посещён, 1=в процессе, 2=завершён
        std::function<bool(uint8_t)> hasCycle = [&](uint8_t node) -> bool {
            if (visited[node] == 1) return true;
            if (visited[node] == 2) return false;
            visited[node] = 1;
            for (uint8_t next : graph[node]) {
                if (hasCycle(next)) return true;
            }
            visited[node] = 2;
            return false;
        };

        for (const auto& kv : graph) {
            if (hasCycle(kv.first)) return "Обнаружен цикл в правилах реле";
        }

        return ""; // OK
    }

} // namespace configJson