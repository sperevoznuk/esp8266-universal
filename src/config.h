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
    String condType; // "sensor", "button", "relay", "time"
    String combine;  // "and" — все условия, "or" — любое условие
    std::vector<SensorCondition> sensors;   // 0-3 для type=="sensor"
    std::vector<uint8_t> sourcePins;        // 1-3 для type=="button" или "relay"
    uint8_t timeFrom = 0;                   // час начала для type=="time" (0-23)
    uint8_t timeTo = 23;                    // час окончания для type=="time" (0-23)
    std::vector<uint8_t> actuators; // Массив пинов реле
};

struct SensorNameEntry {
    String rom;
    String name;
    float offset = 0.0; 
};

struct Config {
    String deviceName;
    String serverUrl;
    uint16_t period = 0; // 0 = ручной режим (автоотправка отключена), иначе период в секундах
    uint32_t configVersion = 0;
    std::vector<Channel> channels; 
    std::vector<Rule> rules;
    std::vector<SensorNameEntry> sensorNames;
};

inline Config g_config;
