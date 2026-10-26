#pragma once
#include "config.h"
#include "common.h"

namespace channels {

    // Объявляем функцию заранее, чтобы apply() могла ее использовать
    inline void setRelay(uint8_t pin, bool state);

    inline void apply() {
        logf("Применение конфигурации каналов...");
        for (const auto& ch : g_config.channels) {
            if (ch.type == "relay") {
                pinMode(ch.pin, OUTPUT);
                // ВАЖНО: Принудительно выключаем реле при старте (с учетом Active LOW)
                setRelay(ch.pin, false); 
                logf("Реле на пине %d инициализировано (ВЫКЛ)", ch.pin);
            } 
            else if (ch.type == "button") {
                pinMode(ch.pin, INPUT_PULLUP);
                logf("Кнопка на пине %d инициализирована", ch.pin);
            }
        }
    }

    inline void setRelay(uint8_t pin, bool state) {
        // Если реле Active LOW (инвертированное):
        // ВКЛ (true) -> LOW, ВЫКЛ (false) -> HIGH
        digitalWrite(pin, state ? LOW : HIGH); 
        // Логируем только если состояние меняется (чтобы не спамить при старте)
        static bool lastState[17] = {false}; // Простейший кэш для 17 пинов ESP8266
        if (lastState[pin] != state) {
            lastState[pin] = state;
            logf("Реле на пине %d -> %s", pin, state ? "ВКЛ" : "ВЫКЛ");
        }
    }

    inline bool getRelayState(uint8_t pin) {
        return digitalRead(pin) == LOW; 
    }

    inline bool getButtonState(uint8_t pin) {
        return digitalRead(pin) == LOW; 
    }
}
