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

        // Сохраняем AP активным, чтобы клиент получил ответ
        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(ssid.c_str(), pass.c_str());

        // Ждём подключения с таймаутом 15 секунд
        unsigned long start = millis();
        bool connected = false;
        while (millis() - start < 15000) {
            // Нужно крутить DNS, чтобы клиент не терял связь с порталом
            dnsServer.processNextRequest();
            delay(200);
            if (WiFi.status() == WL_CONNECTED) {
                connected = true;
                break;
            }
        }

        if (connected) {
            String ip = WiFi.localIP().toString();
            logf("Успешно подключен к %s! IP: %s", ssid.c_str(), ip.c_str());

            // Отправляем ответ клиенту через AP до выключения
            String html = String("") +
                "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
                "<meta http-equiv=\"refresh\" content=\"5;url=http://" + ip + "/\">"
                "<style>"
                "body{font-family:sans-serif;text-align:center;padding:20px;background:#f4f7f6;}"
                ".success{color:#28a745;font-size:24px;}"
                ".ip{font-size:20px;background:#e9ecef;padding:10px;border-radius:5px;display:inline-block;margin:10px 0;}"
                ".spinner{border:4px solid #f3f3f3;border-top:4px solid #28a745;border-radius:50%;width:40px;height:40px;animation:spin 1s linear infinite;margin:20px auto;}"
                "@keyframes spin{0%{transform:rotate(0deg)}100%{transform:rotate(360deg)}}"
                "</style>"
                "</head><body>"
                "<h2 class=\"success\">✓ Подключение успешно!</h2>"
                "<p>IP адрес устройства:</p>"
                "<div class=\"ip\"><a href='http://" + ip + "/'>" + ip + "</a></div>"
                "<div class=\"spinner\"></div>"
                "<p>Перенаправление через 5 секунд...</p>"
                "<p>Если перенаправление не произошло — подключитесь к сети <b>" + ssid + "</b> и перейдите по <a href='http://" + ip + "/'>ссылке</a></p>"
                "<script>setTimeout(function(){window.location.href='http://" + ip + "/';},5000);</script>"
                "</body></html>";

            mainServer->send(200, "text/html; charset=utf-8", html);
            mainServer->client().flush();
            delay(100);

            // Сохраняем во flash и переключаемся в STA
            WiFi.persistent(true);
            WiFi.mode(WIFI_STA);
            WiFi.begin(ssid.c_str(), pass.c_str());
            delay(500);
            ESP.restart();
        } else {
            logf("Ошибка подключения к %s (status=%d)", ssid.c_str(), WiFi.status());
            String html = String("") +
                "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
                "<style>"
                "body{font-family:sans-serif;text-align:center;padding:20px;background:#f4f7f6;}"
                ".error{color:#dc3545;font-size:24px;}"
                ".btn{display:inline-block;padding:10px 20px;background:#007bff;color:white;text-decoration:none;border-radius:5px;font-size:16px;margin-top:15px;}"
                "</style>"
                "</head><body>"
                "<h2 class=\"error\">✗ Ошибка подключения</h2>"
                "<p>Не удалось подключиться к <b>" + ssid + "</b>.</p>"
                "<p>Проверьте правильность пароля и попробуйте снова.</p>"
                "<br><a href=\"/\" class=\"btn\">← Вернуться к настройкам</a>"
                "</body></html>";
            mainServer->send(200, "text/html; charset=utf-8", html);
            // Остаёмся в AP-режиме, без перезагрузки
        }
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
