#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LittleFS.h>
#include <user_interface.h>
#include <ArduinoJson.h>
#include <map>

#include "config.h"
#include "common.h"
#include "webui.h"
#include "wifi_setup.h"
#include "storage.h"
#include "channels.h"
#include "sensors.h"
#include "logic.h" // Подключаем логику
#include "telemetry.h"

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater(true); // serial-лог процесса OTA-обновления

// --- Обработчики страниц и API ---

void handleRoot() {
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

void handleGetSensors() {
    DynamicJsonDocument doc(2048);
    JsonArray arr = doc.to<JsonArray>();
    sensors::listAll(arr);
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void handleGetConfig() {
    DynamicJsonDocument doc(4096);
    configJson::toJson(g_config, doc, /*includeWifi=*/true);
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void handleSaveConfig() {
    if (!server.hasArg("plain")) { server.send(400, "text/plain", "No body"); return; }
    
    DynamicJsonDocument doc(4096);
    if (deserializeJson(doc, server.arg("plain"))) {
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }

    // 1. Парсим во временный Config
    Config newCfg;
    if (!configJson::fromJson(doc, newCfg)) {
        server.send(400, "text/plain", "Неверный формат конфига");
        return;
    }

    // 2. Валидация
    String err = configJson::validate(newCfg);
    if (err.length() > 0) {
        server.send(400, "text/plain", "Ошибка валидации: " + err);
        return;
    }

    // 3. Детекция выключения правил (до замены g_config)
    //    Если правило было enabled, а стало disabled — выключаем его реле.
    for (const auto& newRule : newCfg.rules) {
        for (const auto& oldRule : g_config.rules) {
            if (oldRule.id == newRule.id && oldRule.enabled && !newRule.enabled) {
                for (uint8_t pin : oldRule.actuators) channels::setRelay(pin, false);
            }
        }
    }

    // 4. Применяем новый конфиг
    g_config = newCfg;

    // 5. Сохраняем в LittleFS
    storage::save();

    // 6. Wi-Fi
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


void handleApiStatus() {
    DynamicJsonDocument doc(2048);
    doc["fw"] = FW_VERSION;
    
    // Реле
    JsonArray relays = doc.createNestedArray("relays");
    for (auto& ch : g_config.channels) {
        if (ch.type == "relay") {
            JsonObject r = relays.createNestedObject();
            r["pin"] = ch.pin; r["state"] = channels::getRelayState(ch.pin); r["name"] = ch.name;
        }
    }
    
    // Кнопки
    JsonArray buttons = doc.createNestedArray("buttons");
    for (auto& ch : g_config.channels) {
        if (ch.type == "button") {
            JsonObject b = buttons.createNestedObject();
            b["pin"] = ch.pin; b["pressed"] = channels::getButtonState(ch.pin); b["name"] = ch.name;
        }
    }
    
    // Датчики
    JsonArray temps = doc.createNestedArray("sensors");
    sensors::readAll(temps);
    // Добавляем имена датчиков
    for (JsonObject s : temps) {
        s["name"] = sensors::getNameByRom(s["id"].as<String>());
    }
    
    // Правила (только active)
    JsonArray ruleStates = doc.createNestedArray("rules");
    for (auto& r : g_config.rules) {
        JsonObject ro = ruleStates.createNestedObject();
        ro["id"] = r.id; ro["active"] = r.active;
    }

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void handleApiTelemetry() {
    DynamicJsonDocument doc(4096);
    
    doc["device"] = g_config.deviceName.length() > 0 ? g_config.deviceName : "ESP8266-Universal";
    
    // sensors: все "датчики" (реле, кнопки, DS18B20) с name/value/type
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
    // Температурные датчики — каждый отдельный элемент (из кэша опроса раз в 5 с)
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
    
    // Правила — только id, name, enabled, triggered
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

void handleApiRelay() {
    if (!server.hasArg("plain")) return;
    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, server.arg("plain"))) return;
    channels::setRelay(doc["pin"], doc["state"]);
    server.send(200, "application/json", "{\"status\":\"ok\"}");
}
void handleApiRestart() {
    server.send(200, "application/json", "{\"status\":\"ok\"}");
    delay(500);
    ESP.restart();
}

void handleScan() {
    int n = WiFi.scanComplete();
    if (n == WIFI_SCAN_FAILED) {
        WiFi.scanNetworks(true);
        server.send(200, "application/json", "[]");
        return;
    }
    if (n == WIFI_SCAN_RUNNING) {
        server.send(200, "application/json", "[]");
        return;
    }
    if (n <= 0) {
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

void setup() {
    Serial.begin(115200);
    delay(200);
    logf("=== Universal ESP8266 Firmware %s ===", FW_VERSION);
    
    storage::begin();
    channels::apply();
    sensors::begin();
    wifiSetup::begin(&server);

    server.on("/", handleRoot);
    server.on("/api/config", HTTP_GET, handleGetConfig);
    server.on("/api/config", HTTP_POST, handleSaveConfig);
    server.on("/api/sensors", HTTP_GET, handleGetSensors);
    server.on("/api/status", HTTP_GET, handleApiStatus);
    server.on("/api/telemetry", HTTP_GET, handleApiTelemetry);
    server.on("/api/relay", HTTP_POST, handleApiRelay);
    server.on("/api/scan", handleScan);
    server.on("/api/restart", HTTP_POST, handleApiRestart);
    server.on("/api/telemetry/send", HTTP_POST, telemetry::handleApiTestTelemetry);
    
    // OTA-обновление прошивки: GET /update — служебная форма, POST /update — загрузка файла
    httpUpdater.setup(&server, "/update");
    
    server.onNotFound([]() {
        if (wifiSetup::isConfigMode) {
            server.sendHeader("Location", "http://192.168.4.1/", true);
            server.send(302, "text/plain", "");
        } else {
            server.send(404, "text/plain", "Not Found");
        }
    });
    
    server.begin();
    logf("Web Server запущен на порту 80");
}

void loop() {
    wifiSetup::tick();
    server.handleClient();

    // Неблокирующий опрос датчиков (конверсия раз в 5 с, чтение из кэша)
    sensors::tick();

    // Вызов логики раз в 1 секунду
    static unsigned long lastEval = 0;
    if (millis() - lastEval > 1000) {
        lastEval = millis();
        logic::evaluateRules();
    }

    // Автоматическая отправка телеметрии
    static unsigned long lastTelemetry = 0;
    uint16_t period = g_config.period;
    if (period > 0 && WiFi.status() == WL_CONNECTED) {
        unsigned long interval = (unsigned long)period * 1000UL;
        if (millis() - lastTelemetry > interval) {
            lastTelemetry = millis();
            telemetry::sendTelemetry();
        }
    }

    yield();
}