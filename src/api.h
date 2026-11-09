#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <user_interface.h>
#include "config.h"
#include "common.h"
#include "globals.h"      // Подключаем extern server
#include "config_json.h"
#include "channels.h"
#include "sensors.h"
#include "storage.h"
#include "telemetry.h"
#include "wifi_setup.h"

namespace api {
   inline void handleRoot() {
        if (wifiSetup::isConfigMode) {
            struct station_config conf;
            wifi_station_get_config(&conf);
            String savedSsid = escapeHtml(String((char*)conf.ssid));
            String html = FPSTR(CONFIG_HTML);
            html.replace("%SSID%", savedSsid);
            server.send(200, "text/html; charset=utf-8", html);
        } else {
            server.send(200, "text/html; charset=utf-8", INDEX_HTML);
        }
    }

    inline void handleStyleCss() {
        server.send_P(200, "text/css", WEBUI_CSS);
    }

    inline void handleAppJs() {
        server.send_P(200, "application/javascript", WEBUI_JS);
    }
     
    // --- Датчики ---
    inline void handleGetSensors() {
        DynamicJsonDocument doc(2048);
        JsonArray arr = doc.to<JsonArray>();
        sensors::listAll(arr);
        String out;
        serializeJson(doc, out);
        server.send(200, "application/json", out);
    }

    // --- Конфиг ---
    inline void handleGetConfig() {
        DynamicJsonDocument doc(4096);
        configJson::toJson(g_config, doc, /*includeWifi=*/true);
        String out;
        serializeJson(doc, out);
        server.send(200, "application/json", out);
    }

    inline void handleSaveConfig() {
        if (!server.hasArg("plain")) { server.send(400, "text/plain", "No body"); return; }
        
        DynamicJsonDocument doc(4096);
        if (deserializeJson(doc, server.arg("plain"))) {
            server.send(400, "text/plain", "Invalid JSON");
            return;
        }

        Config newCfg;
        if (!configJson::fromJson(doc, newCfg)) {
            server.send(400, "text/plain", "Неверный формат конфига");
            return;
        }

        String err = configJson::validate(newCfg);
        if (err.length() > 0) {
            server.send(400, "text/plain", "Ошибка валидации: " + err);
            return;
        }

        // Детекция выключения правил
        for (const auto& newRule : newCfg.rules) {
            for (const auto& oldRule : g_config.rules) {
                if (oldRule.id == newRule.id && oldRule.enabled && !newRule.enabled) {
                    for (uint8_t pin : oldRule.actuators) channels::setRelay(pin, false);
                }
            }
        }

        g_config = newCfg;
        storage::save();

        // Wi-Fi
        String ssid = doc["ssid"] | "";
        String pass = doc["pass"] | "";
        bool wifiChanged = false;

        if (ssid.length() > 0) {
            struct station_config staConf;
            wifi_station_get_config(&staConf);
            String currSsid = String((char*)staConf.ssid);
            String currPass = String((char*)staConf.password);
            if (ssid != currSsid || (pass.length() > 0 && pass != currPass)) {
                wifiChanged = true;
                WiFi.persistent(true);
                WiFi.begin(ssid.c_str(), pass.c_str());
                logf("Wi-Fi изменён: SSID=%s", ssid.c_str());
            }
        }

        server.send(200, "application/json", "{\"status\":\"ok\"}");
        if (wifiChanged) {
            delay(1000);
            ESP.restart();
        }
    }

    // --- Статус и Телеметрия ---
    inline void handleApiStatus() {
        DynamicJsonDocument doc(2048);
        doc["fw"] = FW_VERSION;
        
        JsonArray relays = doc.createNestedArray("relays");
        for (auto& ch : g_config.channels) {
            if (ch.type == "relay") {
                JsonObject r = relays.createNestedObject();
                r["pin"] = ch.pin; r["state"] = channels::getRelayState(ch.pin); r["name"] = ch.name;
            }
        }
        
        JsonArray buttons = doc.createNestedArray("buttons");
        for (auto& ch : g_config.channels) {
            if (ch.type == "button") {
                JsonObject b = buttons.createNestedObject();
                b["pin"] = ch.pin; b["pressed"] = channels::getButtonState(ch.pin); b["name"] = ch.name;
            }
        }
        
        JsonArray temps = doc.createNestedArray("sensors");
        sensors::readAll(temps);
        for (JsonObject s : temps) {
            s["name"] = sensors::getNameByRom(s["id"].as<String>());
        }
        
        JsonArray ruleStates = doc.createNestedArray("rules");
        for (auto& r : g_config.rules) {
            JsonObject ro = ruleStates.createNestedObject();
            ro["id"] = r.id; ro["active"] = r.active;
        }

        String out;
        serializeJson(doc, out);
        server.send(200, "application/json", out);
    }

    inline void handleApiTelemetry() {
        DynamicJsonDocument doc(4096);
        doc["device"] = g_config.deviceName.length() > 0 ? g_config.deviceName : "ESP8266-Universal";
        
        JsonArray sArr = doc.createNestedArray("sensors");
        for (auto& ch : g_config.channels) {
            if (ch.type == "relay") {
                JsonObject o = sArr.createNestedObject();
                o["name"] = ch.name.length() > 0 ? ch.name : "D" + String(ch.pin);
                o["value"] = channels::getRelayState(ch.pin);
                o["type"] = "relay";
            } else if (ch.type == "button") {
                JsonObject o = sArr.createNestedObject();
                o["name"] = ch.name.length() > 0 ? ch.name : "D" + String(ch.pin);
                o["value"] = channels::getButtonState(ch.pin);
                o["type"] = "button";
            }
        }
        for (const auto& kv : sensors::tempCache) {
            float temp = kv.second;
            if (temp != -127.0) {
                JsonObject o = sArr.createNestedObject();
                String sn = sensors::getNameByRom(kv.first);
                o["name"] = sn.length() > 0 ? sn : kv.first;
                o["value"] = temp;
                o["type"] = "temperature";
            }
        }
        
        JsonArray rArr = doc.createNestedArray("rules");
        for (auto& r : g_config.rules) {
            JsonObject ro = rArr.createNestedObject();
            ro["id"] = r.id;
            ro["name"] = r.name;
            ro["enabled"] = r.enabled;
            ro["triggered"] = r.active;
        }

        String out;
        serializeJson(doc, out);
        server.send(200, "application/json", out);
    }

    inline void handleApiTestTelemetry() {
        if (g_config.serverUrl.length() == 0) {
            server.send(400, "application/json", "{\"status\":\"error\",\"error\":\"URL сервера не задан\"}");
            return;
        }
        if (WiFi.status() != WL_CONNECTED) {
            server.send(502, "application/json", "{\"status\":\"error\",\"error\":\"Нет подключения к Wi-Fi\"}");
            return;
        }

        String payload = telemetry::buildPayload();
        logf("Телеметрия: проверка, %d байт на %s", payload.length(), g_config.serverUrl.c_str());

        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;
        http.setTimeout(3000);
        http.begin(client, g_config.serverUrl);
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST(payload);
        yield();

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

    // --- Реле ---
    inline void handleApiRelay() {
        if (!server.hasArg("plain")) return;
        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, server.arg("plain"))) return;
        channels::setRelay(doc["pin"], doc["state"]);
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    }

    // --- Служебные ---
    inline void handleApiRestart() {
        server.send(200, "application/json", "{\"status\":\"ok\"}");
        delay(500);
        ESP.restart();
    }

    inline void handleScan() {
        int n = WiFi.scanComplete();
        if (n == WIFI_SCAN_FAILED || n == WIFI_SCAN_RUNNING || n <= 0) {
            if (n == WIFI_SCAN_FAILED) WiFi.scanNetworks(true);
            server.send(200, "application/json", "[]");
            return;
        }
        
        DynamicJsonDocument doc(2048);
        JsonArray arr = doc.to<JsonArray>();
        for (int i = 0; i < n; i++) {
            JsonObject net = arr.createNestedObject();
            net["ssid"] = WiFi.SSID(i);
            net["rssi"] = WiFi.RSSI(i);
            net["open"] = (WiFi.encryptionType(i) == ENC_TYPE_NONE);
        }
        String out;
        serializeJson(doc, out);
        WiFi.scanDelete();
        server.send(200, "application/json", out);
    }

} // namespace api