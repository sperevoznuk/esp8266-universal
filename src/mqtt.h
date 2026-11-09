#pragma once
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "common.h"
#include "channels.h"
#include "sensors.h"
#include "logic.h"

namespace mqtt {

    inline WiFiClient espClient;
    inline PubSubClient client(espClient);

    inline String baseTopic = "";
    inline String deviceId = "";
    inline unsigned long lastReconnectAttempt = 0;

    // Forward declarations
    inline void syncStates();
    inline void connect();
    inline void sendDiscovery();

    // --- Утилита: publish с ретеншном ---
    inline void pub(const String& topic, const String& payload, bool retain = true) {
        if (client.connected()) {
            // Если это Discovery-топик (для HA) — выводим в лог
            if (topic.startsWith("homeassistant/")) {
                logf("MQTT PUB [%s]: %s", topic.c_str(), payload.c_str());
            }
            bool ok = client.publish(topic.c_str(), payload.c_str(), retain);
            if (!ok && topic.startsWith("homeassistant/")) {
                logf("ОШИБКА: Не удалось отправить Discovery (буфер переполнен?)");
            }
        }
    }

    // --- Публикация состояния реле ---
    inline void publishRelayState(uint8_t pin, bool state) {
        pub(baseTopic + "/relay/" + String(pin) + "/state", state ? "ON" : "OFF");
    }

    // --- Публикация состояния кнопки ---
    inline void publishButtonState(uint8_t pin, bool pressed) {
        pub(baseTopic + "/button/" + String(pin) + "/state", pressed ? "PRESSED" : "RELEASED");
    }

    // --- Публикация температур ---
    inline void publishTemperatures() {
        for (const auto& sn : g_config.sensorNames) {
            float t = sensors::getTempByRom(sn.rom);
            if (t == -127.0) continue;
            char buf[16];
            snprintf(buf, sizeof(buf), "%.2f", t);
            pub(baseTopic + "/sensor/" + sn.rom + "/state", String(buf));
        }
    }
    inline void publishRuleState(const Rule& r) {
        String base = baseTopic + "/rule/" + r.id + "/state";
        
        // Для HA (простые строки)
        pub(base + "/enabled", r.enabled ? "ON" : "OFF");
        pub(base + "/active", r.active ? "ON" : "OFF");
        
        // Оставляем JSON для общего использования (телеметрия / отладка)
        DynamicJsonDocument doc(256);
        doc["enabled"] = r.enabled;
        doc["active"]  = r.active;
        doc["name"]    = r.name;
        String out;
        serializeJson(doc, out);
        pub(base, out);
    }

    inline void publishAllRuleStates() {
        for (const auto& r : g_config.rules) publishRuleState(r);
    }

    // --- Callback на входящие команды ---
    inline void callback(char* topic, byte* payload, unsigned int length) {
        String t(topic);
        String msg;
        for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
        logf("MQTT RX [%s]: %s", topic, msg.c_str());

        // /relay/<pin>/set  (payload: ON / OFF)
        if (t.startsWith(baseTopic + "/relay/")) {
            int start = (baseTopic + "/relay/").length();
            int end = t.indexOf("/", start);
            if (end == -1) return;
            uint8_t pin = t.substring(start, end).toInt();
            bool state = (msg == "ON");
            channels::setRelay(pin, state);
            publishRelayState(pin, state);
            return;
        }

        // /rule/<id>/enabled/set  (payload: 1 / 0)
        if (t.startsWith(baseTopic + "/rule/")) {
            String rest = t.substring((baseTopic + "/rule/").length());
            int idx = rest.indexOf("/");
            if (idx == -1) return;
            String ruleId = rest.substring(0, idx);
            if (rest.endsWith("/enabled/set")) {
                bool enabled = (msg == "1" || msg == "ON" || msg == "true");
                for (auto& r : g_config.rules) {
                    if (r.id == ruleId) {
                        r.enabled = enabled;
                        publishRuleState(r);
                        logf("MQTT: правило '%s' -> %s", r.name.c_str(), enabled ? "ВКЛ" : "ВЫКЛ");
                        break;
                    }
                }
            }
            return;
        }
    }

    // --- Отправка Discovery в Home Assistant ---
    inline void sendDiscovery() {
        String deviceName = g_config.deviceName.length() > 0 ? g_config.deviceName : "ESP8266-Universal";
        String ip = WiFi.localIP().toString();

        // Реле → switch
        for (const auto& ch : g_config.channels) {
            if (ch.type != "relay") continue;
            String topic = "homeassistant/switch/" + deviceId + "_relay" + String(ch.pin) + "/config";
            DynamicJsonDocument doc(512);
            doc["name"] = ch.name.length() ? ch.name : ("Реле D" + String(ch.pin));
            doc["unique_id"] = deviceId + "_relay" + String(ch.pin);
            doc["state_topic"] = baseTopic + "/relay/" + String(ch.pin) + "/state";
            doc["command_topic"] = baseTopic + "/relay/" + String(ch.pin) + "/set";
            doc["payload_on"] = "ON";
            doc["payload_off"] = "OFF";
            
            JsonObject dev = doc.createNestedObject("device");
            JsonArray ids = dev.createNestedArray("identifiers");
            ids.add(deviceId);
            dev["name"] = deviceName;
            dev["model"] = "Universal ESP8266";
            dev["configuration_url"] = "http://" + ip;
            
            String out;
            serializeJson(doc, out);
            pub(topic, out);
        }

        // Кнопки → binary_sensor
        for (const auto& ch : g_config.channels) {
            if (ch.type != "button") continue;
            String topic = "homeassistant/binary_sensor/" + deviceId + "_button" + String(ch.pin) + "/config";
            DynamicJsonDocument doc(512);
            doc["name"] = ch.name.length() ? ch.name : ("Кнопка D" + String(ch.pin));
            doc["unique_id"] = deviceId + "_button" + String(ch.pin);
            doc["state_topic"] = baseTopic + "/button/" + String(ch.pin) + "/state";
            doc["payload_on"] = "PRESSED";
            doc["payload_off"] = "RELEASED";
            
            JsonObject dev = doc.createNestedObject("device");
            JsonArray ids = dev.createNestedArray("identifiers");
            ids.add(deviceId);
            dev["name"] = deviceName;
            dev["model"] = "Universal ESP8266";
            dev["configuration_url"] = "http://" + ip;
            
            String out;
            serializeJson(doc, out);
            pub(topic, out);
        }

        // Датчики → sensor
        for (const auto& sn : g_config.sensorNames) {
            String topic = "homeassistant/sensor/" + deviceId + "_sensor_" + sn.rom + "/config";
            DynamicJsonDocument doc(512);
            doc["name"] = sn.name.length() ? sn.name : sn.rom;
            doc["unique_id"] = deviceId + "_temp_" + sn.rom;
            doc["state_topic"] = baseTopic + "/sensor/" + sn.rom + "/state";
            doc["unit_of_measurement"] = "°C";
            doc["device_class"] = "temperature";
            doc["state_class"] = "measurement";
            
            JsonObject dev = doc.createNestedObject("device");
            JsonArray ids = dev.createNestedArray("identifiers");
            ids.add(deviceId);
            dev["name"] = deviceName;
            dev["model"] = "Universal ESP8266";
            dev["configuration_url"] = "http://" + ip;
            
            String out;
            serializeJson(doc, out);
            pub(topic, out);
        }

        // Правила: Switch (вкл/выкл enabled) + Binary Sensor (сработало active)
                // Правила: Switch (вкл/выкл enabled) + Binary Sensor (сработало active)
        for (const auto& r : g_config.rules) {
            String ruleName = r.name.length() ? r.name : ("Правило " + r.id);
            
            // --- Формируем читаемое описание условия ---
            String condDesc = "";
            if (r.condType == "sensor") {
                for (size_t i = 0; i < r.sensors.size(); i++) {
                    if (i > 0) condDesc += (r.combine == "or") ? " ИЛИ " : "; ";
                    const auto& sc = r.sensors[i];
                    String sname = sensors::getNameByRom(sc.rom);
                    if (sname.length() == 0) sname = sc.rom;
                    String op = sc.mode ? "≤" : "≥";
                    condDesc += sname + " " + op + String(sc.target, 1) + "°C";
                }
            } else if (r.condType == "button" || r.condType == "relay") {
                String typeLabel = (r.condType == "relay") ? "Реле" : "Кнопка";
                for (size_t i = 0; i < r.sourcePins.size(); i++) {
                    if (i > 0) condDesc += (r.combine == "or") ? " ИЛИ " : "; ";
                    condDesc += typeLabel + " " + String(r.sourcePins[i]);
                }
            }

            // Собираем финальное имя для HA
            String haName = ruleName;
            if (condDesc.length() > 0) {
                haName += " (" + condDesc + ")";
            }

            // 1. Switch (управление)
            String switchTopic = "homeassistant/switch/" + deviceId + "_rule_" + r.id + "/config";
            {
                DynamicJsonDocument doc(768); // Увеличили буфер под длинные имена
                doc["name"] = haName;
                doc["unique_id"] = deviceId + "_rule_" + r.id;
                doc["state_topic"] = baseTopic + "/rule/" + r.id + "/state/enabled";
                doc["command_topic"] = baseTopic + "/rule/" + r.id + "/enabled/set";
                doc["payload_on"] = "ON";
                doc["payload_off"] = "OFF";
                
                JsonObject dev = doc.createNestedObject("device");
                JsonArray ids = dev.createNestedArray("identifiers");
                ids.add(deviceId);
                dev["name"] = deviceName;
                dev["model"] = "Universal ESP8266";
                dev["configuration_url"] = "http://" + ip;
                
                String out;
                serializeJson(doc, out);
                pub(switchTopic, out);
            }

            // 2. Binary Sensor (Сработало)
            String sensorTopic = "homeassistant/binary_sensor/" + deviceId + "_rule_active_" + r.id + "/config";
            {
                DynamicJsonDocument doc(768);
                doc["name"] = haName + " (Сработало)";
                doc["unique_id"] = deviceId + "_rule_active_" + r.id;
                doc["state_topic"] = baseTopic + "/rule/" + r.id + "/state/active";
                doc["payload_on"] = "ON";
                doc["payload_off"] = "OFF";
                doc["device_class"] = "power";
                
                JsonObject dev = doc.createNestedObject("device");
                JsonArray ids = dev.createNestedArray("identifiers");
                ids.add(deviceId);
                dev["name"] = deviceName;
                dev["model"] = "Universal ESP8266";
                dev["configuration_url"] = "http://" + ip;
                
                String out;
                serializeJson(doc, out);
                pub(sensorTopic, out);
            }
        }
    }

    // --- Подключение к брокеру и подписка ---
    inline void connect() {
        if (!g_config.mqttEnabled) return;
        if (g_config.mqttServer.length() == 0) return;
        if (WiFi.status() != WL_CONNECTED) return;

        String clientId = deviceId;
        logf("MQTT: подключение к %s:%d...", g_config.mqttServer.c_str(), g_config.mqttPort);

        String lwtTopic = baseTopic + "/status";
        bool ok;
        if (g_config.mqttUser.length() > 0) {
            ok = client.connect(clientId.c_str(), g_config.mqttUser.c_str(), g_config.mqttPass.c_str(), lwtTopic.c_str(), 1, true, "offline");
        } else {
            ok = client.connect(clientId.c_str(), lwtTopic.c_str(), 1, true, "offline");
        }

        if (!ok) {
            logf("MQTT: ошибка подключения, rc=%d", client.state());
            return;
        }
        logf("MQTT: подключено");

        pub(lwtTopic, "online");
        client.subscribe((baseTopic + "/relay/+/set").c_str());
        client.subscribe((baseTopic + "/rule/+/enabled/set").c_str());

        sendDiscovery();
        publishAllRuleStates();
        for (const auto& ch : g_config.channels) {
            if (ch.type == "relay") publishRelayState(ch.pin, channels::getRelayState(ch.pin));
            if (ch.type == "button") publishButtonState(ch.pin, channels::getButtonState(ch.pin));
        }
        publishTemperatures();
    }

    // --- Синхронизация состояний (вызывается из process) ---
    inline void syncStates() {
        // Реле
        static bool lastRelayState[17] = {false};
        for (const auto& ch : g_config.channels) {
            if (ch.type == "relay") {
                bool cur = channels::getRelayState(ch.pin);
                if (cur != lastRelayState[ch.pin]) {
                    lastRelayState[ch.pin] = cur;
                    publishRelayState(ch.pin, cur);
                }
            }
        }
        // Правила
        static String lastRuleActive = "";
        String curHash;
        for (const auto& r : g_config.rules) {
            curHash += r.enabled ? '1' : '0';
            curHash += r.active ? '1' : '0';
        }
        if (curHash != lastRuleActive) {
            lastRuleActive = curHash;
            publishAllRuleStates();
        }
    }
   // Внутри namespace mqtt
    inline void process() {
        if (!g_config.mqttEnabled) return;
        
        if (!client.connected()) {
            unsigned long now = millis();
            if (now - lastReconnectAttempt > 5000) {
                lastReconnectAttempt = now;
                connect();
            }
        } else {
            client.loop();
            
            // Публикуем температуру раз в 5 секунд
            static unsigned long lastTempPub = 0;
            if (millis() - lastTempPub > 5000) {
                lastTempPub = millis();
                publishTemperatures(); // Вызываем уже существующую функцию
            }
        }
    }
 
    // --- Запуск MQTT ---
    inline void begin() {
        uint8_t mac[6];
        WiFi.macAddress(mac);
        char suffix[8];
        snprintf(suffix, sizeof(suffix), "%02X%02X%02X", mac[3], mac[4], mac[5]);
        deviceId = String("esp-") + suffix;
        baseTopic = "espuniversal/" + deviceId;

        client.setServer(g_config.mqttServer.c_str(), g_config.mqttPort);
        client.setCallback(callback);
        client.setBufferSize(2048); // Увеличили для Discovery

        if (g_config.mqttEnabled && g_config.mqttServer.length() > 0) {
            logf("MQTT: брокер %s:%d, топик %s", g_config.mqttServer.c_str(), g_config.mqttPort, baseTopic.c_str());
        }
    }

     

} // namespace mqtt