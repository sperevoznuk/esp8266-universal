#pragma once
#include "config.h"
#include "channels.h"
#include "sensors.h"
#include "common.h"

namespace logic {

    // Оценка одного датчика с собственным гистерезисом
    inline bool evaluateSensorCond(SensorCondition& sc) {
        float t = sensors::getTempByRom(sc.rom);
        if (t == -127.0) {
            // Fail-safe: если датчик умер, считаем условие выполненным
            return true;
        }

        if (sc.mode) {
            // "≤" режим: включает когда t опустилась до target-hyst, выключает когда t поднялась до target+hyst
            if (sc.active) {
                return t < (sc.target + sc.hysteresis);
            } else {
                return t <= (sc.target - sc.hysteresis);
            }
        } else {
            // "≥" режим: включает когда t поднялась до target+hyst, выключает когда t опустилась до target-hyst
            if (sc.active) {
                return t > (sc.target - sc.hysteresis);
            } else {
                return t >= (sc.target + sc.hysteresis);
            }
        }
    }

    // Вычисление одного правила
    inline bool evaluateCondition(const Rule& r) {
        bool isOr = (r.combine == "or");

        if (r.condType == "sensor") {
            Rule& rw = const_cast<Rule&>(r);
            for (auto& sc : rw.sensors) {
                bool satisfied = evaluateSensorCond(sc);
                sc.active = satisfied;
                if (isOr) {
                    if (satisfied) return true;  // OR: достаточно одного
                } else {
                    if (!satisfied) return false; // AND: все должны быть
                }
            }
            return isOr ? false : !r.sensors.empty();
        } 
        else if (r.condType == "button") {
            if (isOr) {
                for (uint8_t pin : r.sourcePins) {
                    if (channels::getButtonState(pin)) return true;
                }
                return false;
            } else {
                for (uint8_t pin : r.sourcePins) {
                    if (!channels::getButtonState(pin)) return false;
                }
                return !r.sourcePins.empty();
            }
        } 
        else if (r.condType == "relay") {
            if (isOr) {
                for (uint8_t pin : r.sourcePins) {
                    if (channels::getRelayState(pin)) return true;
                }
                return false;
            } else {
                for (uint8_t pin : r.sourcePins) {
                    if (!channels::getRelayState(pin)) return false;
                }
                return !r.sourcePins.empty();
            }
        }
        return false;
    }

    // Применение состояний ко всем реле по правилам (OR-логика)
    // Возвращает true, если хоть одно реле изменило состояние
    inline bool applyRelayStates() {
        bool anyChanged = false;
        for (auto& ch : g_config.channels) {
            if (ch.type != "relay") continue;
            bool shouldBeOn = false;
            bool hasRules = false;
            for (const auto& r : g_config.rules) {
                if (!r.enabled) continue;
                bool refsThis = false;
                for (uint8_t actPin : r.actuators) {
                    if (actPin == ch.pin) { refsThis = true; break; }
                }
                if (!refsThis) continue;
                hasRules = true;
                if (r.active) { shouldBeOn = true; break; }
            }
            if (hasRules) {
                channels::setRelay(ch.pin, shouldBeOn);
                anyChanged = true;
            }
        }
        return anyChanged;
    }

    // Основной цикл вычисления правил (DAG)
    inline void evaluateRules() {
        // 0. Детектор перехода enabled→disabled: выключить актуаторы
        for (auto& r : g_config.rules) {
            if (r.prevEnabled && !r.enabled) {
                for (uint8_t pin : r.actuators) {
                    channels::setRelay(pin, false);
                }
            }
            r.prevEnabled = r.enabled;
        }

        // 1. Сбрасываем active у выключенных правил
        for (auto& r : g_config.rules) {
            if (!r.enabled) r.active = false;
        }

        // 2. Итеративно разрешаем каскады:
        //    оцениваем правила и применяем реле в каждом проходе,
        //    чтобы следующие проходы видели обновлённые состояния реле.
        for (int depth = 0; depth < MAX_CASCADE_DEPTH; depth++) {
            bool changed = false;

            // Оцениваем все правила
            for (auto& r : g_config.rules) {
                if (!r.enabled) continue;
                bool newActive = evaluateCondition(r);
                if (newActive != r.active) {
                    r.active = newActive;
                    changed = true;
                }
            }

            // Применяем реле — следующий проход увидит новые состояния
            applyRelayStates();

            if (!changed) break;
        }
    }
}