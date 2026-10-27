#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
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

ESP8266WebServer server(80);

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
    StaticJsonDocument<512> doc;
    JsonArray arr = doc.to<JsonArray>();
    sensors::listAll(arr);
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void handleGetConfig() {
    DynamicJsonDocument doc(4096);
    doc["deviceName"] = g_config.deviceName;
    doc["serverUrl"] = g_config.serverUrl;
    doc["period"] = g_config.period;
    
    // Текущий сохраненный SSID и пароль из флеша ESP
    struct station_config conf;
    wifi_station_get_config(&conf);
    doc["ssid"] = String((char*)conf.ssid);
    doc["pass"] = String((char*)conf.password);
    
    JsonArray chArr = doc.createNestedArray("channels");
    for (auto& ch : g_config.channels) {
        JsonObject c = chArr.createNestedObject();
        c["type"] = ch.type; c["pin"] = ch.pin; c["name"] = ch.name;
    }
    
    JsonArray rArr = doc.createNestedArray("rules");
    for (auto& r : g_config.rules) {
        JsonObject jr = rArr.createNestedObject();
        jr["id"] = r.id; jr["enabled"] = r.enabled; jr["name"] = r.name;
        JsonObject cond = jr.createNestedObject("cond");
        cond["type"] = r.cond.type;
        if (r.cond.type == "sensor") {
            cond["rom"] = r.cond.rom;
            cond["target"] = r.target;
            cond["hysteresis"] = r.hysteresis;
        } else {
            cond["pin"] = r.cond.pin;
        }
        JsonArray acts = jr.createNestedArray("actuators");
        for (uint8_t a : r.actuators) acts.add(a);
    }
    
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

// Валидация конфига на наличие циклов и битых ссылок
bool validateConfig(const DynamicJsonDocument& doc) {
    JsonArrayConst chArr = doc["channels"];
    JsonArrayConst rArr = doc["rules"];
    
    // 1. Проверка уникальности пинов каналов
    std::vector<int> pins;
    for (JsonVariantConst c : chArr) {
        int p = c["pin"];
        if (std::find(pins.begin(), pins.end(), p) != pins.end()) return false;
        pins.push_back(p);
    }

    // 2. Проверка ссылок в правилах и поиск циклов
    // Строим граф зависимостей: source_pin -> [actuator_pins]
    std::map<int, std::vector<int>> graph;
    
    for (JsonVariantConst r : rArr) {
        if (r["cond"]["type"] == "relay") {
            int src = r["cond"]["pin"];
            JsonArrayConst acts = r["actuators"];
            for (JsonVariantConst act : acts) {
                int dst = act;
                // Проверяем, что dst - это существующее реле
                bool isRelay = false;
                for (JsonVariantConst c : chArr) {
                    if (c["pin"] == dst && c["type"] == "relay") isRelay = true;
                }
                if (!isRelay) return false;
                
                graph[src].push_back(dst);
            }
        }
    }

    // Поиск циклов (DFS)
    std::map<int, int> visited; // 0 - не посещен, 1 - в процессе, 2 - завершен
    std::function<bool(int)> hasCycle = [&](int node) {
        if (visited[node] == 1) return true;
        if (visited[node] == 2) return false;
        visited[node] = 1;
        for (int next : graph[node]) {
            if (hasCycle(next)) return true;
        }
        visited[node] = 2;
        return false;
    };

    for (auto const& [node, edges] : graph) {
        if (hasCycle(node)) return false;
    }

    return true;
}

void handleSaveConfig() {
    if (!server.hasArg("plain")) { server.send(400, "text/plain", "No body"); return; }
    
    DynamicJsonDocument doc(4096);
    if (deserializeJson(doc, server.arg("plain"))) { server.send(400, "text/plain", "Invalid JSON"); return; }

    if (!validateConfig(doc)) {
        server.send(400, "text/plain", "Ошибка валидации: проверьте уникальность пинов и отсутствие циклов в правилах.");
        return;
    }

    // Если валидация прошла, сохраняем
    g_config.channels.clear();
    g_config.rules.clear();
    
    g_config.deviceName = doc["deviceName"] | "";
    g_config.serverUrl = doc["serverUrl"] | "";
    g_config.period = doc["period"] | 60;
    
    for (JsonVariantConst c : doc["channels"].as<JsonArrayConst>()) {
        Channel ch; ch.type = c["type"].as<String>(); ch.pin = c["pin"]; ch.name = c["name"].as<String>();
        g_config.channels.push_back(ch);
    }
    
    for (JsonVariantConst r : doc["rules"].as<JsonArrayConst>()) {
        Rule rule;
        rule.id = r["id"].as<String>(); rule.enabled = r["enabled"]; rule.name = r["name"].as<String>();
        rule.cond.type = r["cond"]["type"].as<String>();
        if (rule.cond.type == "sensor") {
            rule.cond.rom = r["cond"]["rom"].as<String>();
            rule.target = r["cond"]["target"]; rule.hysteresis = r["cond"]["hysteresis"];
        } else {
            rule.cond.pin = r["cond"]["pin"];
        }
        JsonArrayConst acts = r["actuators"];
        for (JsonVariantConst a : acts) rule.actuators.push_back(a);
        g_config.rules.push_back(rule);
    }

    storage::save();
    
    // Применяем Wi-Fi настройки только если они изменились
    String ssid = doc["ssid"] | "";
    String pass = doc["pass"] | "";
    bool wifiChanged = false;
    
    if (ssid.length() > 0) {
        struct station_config staConf;
        wifi_station_get_config(&staConf);
        String currSsid = String((char*)staConf.ssid);
        String currPass = String((char*)staConf.password);
        
        // Кредалы изменились — применяем
        if (ssid != currSsid || (pass.length() > 0 && pass != currPass)) {
            wifiChanged = true;
            WiFi.persistent(true);
            WiFi.begin(ssid.c_str(), pass.c_str());
            logf("Wi-Fi изменён: SSID=%s", ssid.c_str());
        } else {
            // Тот же SSID и пустой пароль — не трогаем Wi-Fi
            logf("Wi-Fi не изменён, перезагрузка не требуется");
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
    // Температурные датчики — каждый отдельный элемент
    for (auto& b : sensors::buses) {
        b.dt->requestTemperatures();
        int count = b.dt->getDeviceCount();
        for (int i = 0; i < count; i++) {
            DeviceAddress addr;
            if (b.dt->getAddress(addr, i)) {
                float temp = b.dt->getTempC(addr);
                if (temp != DEVICE_DISCONNECTED_C) {
                    char romStr[17];
                    for (int j = 0; j < 8; j++) {
                        sprintf(romStr + (j * 2), "%02x", addr[j]);
                    }
                    JsonObject o = sArr.createNestedObject();
                    o["name"] = romStr;
                    o["value"] = temp;
                    o["type"] = "temperature";
                }
            }
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

    // Вызов логики раз в 1 секунду
    static unsigned long lastEval = 0;
    if (millis() - lastEval > 1000) {
        lastEval = millis();
        logic::evaluateRules();
    }

    yield();
}