#pragma once
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <user_interface.h>
#include "config.h"
#include "common.h"

namespace wifiSetup {

    inline DNSServer dnsServer;
    inline const byte DNS_PORT = 53;
    
    inline bool isConfigMode = false;
    inline bool wifiReady = false;
    inline String apName;
    inline unsigned long disconnectStartTime = 0;
    inline unsigned long lastWifiCheck = 0;
    
    // Указатель на основной сервер, чтобы модуль мог навешивать свои маршруты
    inline ESP8266WebServer* mainServer = nullptr;

    // Обработчик сохранения формы (перенесен сюда)
    inline void handleSave() {
        String ssid = mainServer->arg("ssid");
        String pass = mainServer->arg("pass");
        logf("Получена форма настройки. SSID: %s", ssid.c_str());
        
        mainServer->send(200, "text/html; charset=utf-8", "<h2>Сохранено!</h2><p>Перезагрузка...</p>");
        
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), pass.c_str());
        delay(2000);
        ESP.restart();
    }

    inline void startConfigPortal() {
        if (isConfigMode) return;
        isConfigMode = true;
        disconnectStartTime = 0;
        logf("Запуск режима настройки (AP Mode)...");
        
        WiFi.mode(WIFI_AP_STA); 
        if (WiFi.SSID().length() > 0) WiFi.begin(); 

        WiFi.softAP(apName.c_str()); 
        IPAddress apIP = WiFi.softAPIP();
        logf("AP IP: %s (Открытая сеть)", apIP.toString().c_str());

        dnsServer.start(DNS_PORT, "*", apIP);
        logf("Портал запущен. Ждем настройки (192.168.4.1)");
    }

    inline void begin(ESP8266WebServer* server) {
        mainServer = server;
        
        // Регистрируем маршрут сохранения формы сразу при старте
        mainServer->on("/save", handleSave);

        // Формируем имя точки доступа
        uint8_t mac[6];
        WiFi.macAddress(mac);
        char suffix[8];
        snprintf(suffix, sizeof(suffix), "%02X%02X%02X", mac[3], mac[4], mac[5]);
        apName = String(AP_NAME_PREFIX) + "-" + suffix;

        // Настройки для стабильности
        WiFi.persistent(true); 
        WiFi.setSleepMode(WIFI_NONE_SLEEP);
        WiFi.mode(WIFI_STA);
        
        logf("Попытка подключения к сохраненной сети...");
        WiFi.begin();

        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
            delay(500);
            Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            wifiReady = true;
            logf("Wi-Fi OK. SSID=%s  IP=%s", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
        } else {
            disconnectStartTime = millis();
            String savedSSID = WiFi.SSID();
            if (savedSSID.length() == 0) {
                startConfigPortal();
            } else {
                logf("Не удалось подключиться к %s. Ждем...", savedSSID.c_str());
            }
        }
    }

    inline void tick() {
        // Если мы в режиме настройки, крутим DNS
        if (isConfigMode) {
            dnsServer.processNextRequest();
            
            // Если в фоне подключились к роутеру
            if (WiFi.status() == WL_CONNECTED) {
                logf("Wi-Fi подключен в фоне! IP=%s. Отключаем AP.", WiFi.localIP().toString().c_str());
                WiFi.mode(WIFI_STA);
                isConfigMode = false;
                wifiReady = true;
            }
        } 
        // Если мы не в сети и не в режиме настройки
        else if (!wifiReady) {
            if (millis() - lastWifiCheck >= WIFI_CHECK_INTERVAL_MS) {
                lastWifiCheck = millis();
                logf("Попытка реконнекта...");
                WiFi.reconnect();
            }
            if (WiFi.status() == WL_CONNECTED) {
                wifiReady = true;
                logf("Wi-Fi восстановлен. IP=%s", WiFi.localIP().toString().c_str());
            }
            // Таймаут на поднятие портала
            if (disconnectStartTime > 0 && (millis() - disconnectStartTime >= PORTAL_START_TIMEOUT_MS)) {
                startConfigPortal();
            }
        }
    }

} // namespace wifiSetup
