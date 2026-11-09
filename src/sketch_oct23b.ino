#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include "config.h"
#include "globals.h"
#include "common.h"
#include "webui.h"
#include "wifi_setup.h"
#include "storage.h"
#include "channels.h"
#include "sensors.h"
#include "logic.h"
#include "telemetry.h"
#include "api.h" // <-- Наши обработчики

// Глобальный экземпляр сервера (объявлен как extern в globals.h)
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater(true);

void setup() {
    Serial.begin(115200);
    delay(200);
    logf("=== Universal ESP8266 Firmware %s ===", FW_VERSION);
    
    storage::begin();
    channels::apply();
    wifiSetup::begin(&server);

    sensors::begin();

    // Регистрация маршрутов
    server.on("/", api::handleRoot);
    server.on("/style.css", HTTP_GET, api::handleStyleCss); 
    server.on("/app.js", HTTP_GET, api::handleAppJs);        
    server.on("/api/config", HTTP_GET, api::handleGetConfig);
    server.on("/api/config", HTTP_POST, api::handleSaveConfig);
    server.on("/api/sensors", HTTP_GET, api::handleGetSensors);
    server.on("/api/status", HTTP_GET, api::handleApiStatus);
    server.on("/api/telemetry", HTTP_GET, api::handleApiTelemetry);
    server.on("/api/relay", HTTP_POST, api::handleApiRelay);
    server.on("/api/scan", api::handleScan);
    server.on("/api/restart", HTTP_POST, api::handleApiRestart);
    server.on("/api/telemetry/send", HTTP_POST, api::handleApiTestTelemetry);
    
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
    sensors::tick();

    static unsigned long lastEval = 0;
    if (millis() - lastEval > 1000) {
        lastEval = millis();
        logic::evaluateRules();
    }

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