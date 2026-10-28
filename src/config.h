#pragma once
#include <Arduino.h>
#include <vector>

#define FW_VERSION          "1.0.0-stage3"
#define AP_NAME_PREFIX      "ESP8266-Setup"
#define WIFI_CHECK_INTERVAL_MS  10000UL
#define PORTAL_START_TIMEOUT_MS 60000UL 
#define MAX_RULES           16
#define MAX_CASCADE_DEPTH   3

struct Channel {
    String type; // "relay", "button", "sensor"
    uint8_t pin; 
    String name; 
};

struct Condition {
    String type;    // "sensor", "button", "relay"
    String rom;     // Только для sensor
    uint8_t pin;    // Только для button/relay
};

struct SensorCondition {
    String rom;
    float target;
    float hysteresis;
    bool mode;          // true = "+" (нагрев), false = "-" (охлаждение)
    bool active = false; // Runtime: гистерезис каждого датчика (не сериализуется)
};

struct Rule {
    String id;
    bool enabled;   // Задается пользователем
    bool active;    // Вычисляется прошивкой (в JSON не сохраняется)
    bool prevEnabled = true; // Предыдущее состояние enabled (runtime, не сериализуется)
    String name;
    String condType; // "sensor", "button", "relay"
    String combine;  // "and" — все условия, "or" — любое условие
    std::vector<SensorCondition> sensors;   // 0-3 для type=="sensor"
    std::vector<uint8_t> sourcePins;        // 1-3 для type=="button" или "relay"
    std::vector<uint8_t> actuators; // Массив пинов реле
};

struct SensorNameEntry {
    String rom;
    String name;
};

struct Config {
    String deviceName;
    String serverUrl;
    uint16_t period = 60; // период отправки телеметрии в секундах
    std::vector<Channel> channels; 
    std::vector<Rule> rules;
    std::vector<SensorNameEntry> sensorNames;
};

inline Config g_config;
