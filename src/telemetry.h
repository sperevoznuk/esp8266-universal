#pragma once
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "common.h"
#include "sensors.h"

extern ESP8266WebServer server;

namespace telemetry {

    // Построение JSON-тела телеметрии в новом формате
    // {"sensors":[{"id":"D5","value":1},{"id":"28a4...","value":17.06}],"rules":[{"id":"...","enabled":1,"triggered":0}]}
    inline String buildPayload() {
        String json = "{\"sensors\":[";
        bool first = true;
        
        // Каналы: relay/button → id = D1-D7, value = 1/0
        for (auto& ch : g_config.channels) {
            if (ch.type == "sensor") continue;
            String id;
            switch (ch.pin) {
                case 5: id = "D1"; break;
                case 4: id = "D2"; break;
                case 0: id = "D3"; break;
                case 2: id = "D4"; break;
                case 14: id = "D5"; break;
                case 12: id = "D6"; break;
                case 13: id = "D7"; break;
                default: id = "D" + String(ch.pin);
            }
            int val = (ch.type == "relay" || ch.type == "button") ? (digitalRead(ch.pin) == LOW ? 1 : 0) : 0;
            if (!first) json += ",";
            first = false;
            json += "{\"id\":\"" + id + "\",\"value\":" + String(val) + "}";
        }

        // Температурные датчики
        for (auto& sn : g_config.sensorNames) {
            float temp = sensors::getTempByRom(sn.rom);
            if (temp != -127.0f) {
                char buf[16];
                snprintf(buf, sizeof(buf), "%.2f", temp);
                if (!first) json += ",";
                first = false;
                json += "{\"id\":\"" + sn.rom + "\",\"value\":" + String(buf) + "}";
            }
        }

        json += "],\"rules\":[";
        first = true;
        for (auto& r : g_config.rules) {
            if (!first) json += ",";
            first = false;
            json += "{\"id\":\"" + r.id + "\",\"enabled\":" + String(r.enabled ? 1 : 0) + ",\"triggered\":" + String(r.active ? 1 : 0) + "}";
        }

        json += "],\"config_version\":" + String(g_config.configVersion) + "}";
        return json;
    }

    // Отправка телеметрии POST-запросом на сервер
    inline bool sendTelemetry() {
        if (g_config.serverUrl.length() == 0) {
            logf("Телеметрия: URL сервера не задан");
            return false;
        }
        if (WiFi.status() != WL_CONNECTED) {
            logf("Телеметрия: нет подключения к Wi-Fi");
            return false;
        }

        String payload = buildPayload();
        logf("Телеметрия: отправка %d байт на %s", payload.length(), g_config.serverUrl.c_str());

        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;
        http.begin(client, g_config.serverUrl);
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST(payload);

        bool success = false;
        if (httpCode > 0) {
            logf("Телеметрия: ответ %d — %s", httpCode, http.getString().c_str());
            success = (httpCode >= 200 && httpCode < 300);
        } else {
            logf("Телеметрия: ошибка %s", http.errorToString(httpCode).c_str());
        }

        http.end();
        return success;
    }

    // Обработчик кнопки "Проверить" — POST /api/telemetry/send
    inline void handleApiTestTelemetry() {
        // Проверки
        if (g_config.serverUrl.length() == 0) {
            server.send(400, "application/json", "{\"status\":\"error\",\"error\":\"URL сервера не задан\"}");
            return;
        }
        if (WiFi.status() != WL_CONNECTED) {
            server.send(502, "application/json", "{\"status\":\"error\",\"error\":\"Нет подключения к Wi-Fi\"}");
            return;
        }

        String payload = buildPayload();
        logf("Телеметрия: проверка, %d байт на %s", payload.length(), g_config.serverUrl.c_str());

        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;
        http.setTimeout(3000);
        http.begin(client, g_config.serverUrl);
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST(payload);
        yield();

        // Формируем ответ
        DynamicJsonDocument result(2048);
        if (httpCode > 0) {
            String body = http.getString();
            yield();
            logf("Телеметрия: ответ %d — %s", httpCode, body.c_str());
            result["status"] = (httpCode >= 200 && httpCode < 300) ? "ok" : "error";
            result["httpCode"] = httpCode;
            result["response"] = body;
            if (httpCode >= 400 && body.length() > 0) result["error"] = body;
        } else {
            logf("Телеметрия: ошибка %s", http.errorToString(httpCode).c_str());
            result["status"] = "error";
            result["error"] = http.errorToString(httpCode);
            result["response"] = "";
        }
        http.end();

        String out;
        serializeJson(result, out);
        server.send(200, "application/json", out);
    }


}
