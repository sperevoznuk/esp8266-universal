#pragma once
#include <map>
#include "config.h"
#include "common.h"

namespace channels {

    // Объявляем функцию заранее, чтобы apply() могла ее использовать
    inline bool setRelay(uint8_t pin, bool state);

    // Кэш последнего записанного состояния реле (pin -> state)
    // Нужен для логирования только реальных изменений и для applyRelayStates.
    // ВАЖНО: source of truth — цифровой выход, но чтение digitalRead()
    // на OUTPUT-пине ESP8266 возвращает записанное значение, так что кэш
    // здесь только для дедупликации логов и детекта изменений.
    inline std::map<uint8_t, bool> relayStateCache;

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

    // Возвращает true, если состояние реально изменилось
    inline bool setRelay(uint8_t pin, bool state) {
        // Если реле Active LOW (инвертированное):
        // ВКЛ (true) -> LOW, ВЫКЛ (false) -> HIGH
        digitalWrite(pin, state ? LOW : HIGH);

        auto it = relayStateCache.find(pin);
        bool changed = (it == relayStateCache.end()) || (it->second != state);
        if (changed) {
            relayStateCache[pin] = state;
            logf("Реле на пине %d -> %s", pin, state ? "ВКЛ" : "ВЫКЛ");
        }
        return changed;
    }

    inline bool getRelayState(uint8_t pin) {
        return digitalRead(pin) == LOW; 
    }

    inline bool getButtonState(uint8_t pin) {
        return digitalRead(pin) == LOW; 
    }
}