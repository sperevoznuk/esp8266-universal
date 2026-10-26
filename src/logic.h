#pragma once
#include "config.h"
#include "channels.h"
#include "sensors.h"
#include "common.h"

namespace logic {

    // Вычисление одного правила
    inline bool evaluateCondition(const Rule& r) {
        if (r.cond.type == "sensor") {
            float t = sensors::getTempByRom(r.cond.rom);
            if (t == -127.0) {
                // Fail-safe: если датчик умер, греем (включаем)
                return true; 
            }
            
            // Гистерезис: 
            // Если правило было активно, выключаем только когда t >= target + h
            // Если правило было неактивно, включаем когда t <= target - h
            if (r.active) {
                return t < (r.target + r.hysteresis);
            } else {
                return t <= (r.target - r.hysteresis);
            }
        } 
        else if (r.cond.type == "button") {
            return channels::getButtonState(r.cond.pin);
        } 
        else if (r.cond.type == "relay") {
            return channels::getRelayState(r.cond.pin);
        }
        return false;
    }

    // Основной цикл вычисления правил (DAG)
    inline void evaluateRules() {
        // 1. Сбрасываем active у выключенных правил
        for (auto& r : g_config.rules) {
            if (!r.enabled) r.active = false;
        }

        // 2. Делаем 3 прохода для разрешения каскадов (глубина 3)
        for (int depth = 0; depth < MAX_CASCADE_DEPTH; depth++) {
            bool changed = false;
            
            for (auto& r : g_config.rules) {
                if (!r.enabled) continue;
                
                bool newActive = evaluateCondition(r);
                if (newActive != r.active) {
                    r.active = newActive;
                    changed = true;
                }
            }
            
            // Если за проход ничего не изменилось, дальше можно не крутить
            if (!changed) break; 
        }

        // 3. Применяем состояния к реле (OR-логика)
        for (const auto& ch : g_config.channels) {
            if (ch.type != "relay") continue;

            bool shouldBeOn = false;
            
            // Проверяем, требует ли какое-либо активное правило включить это реле
            for (const auto& r : g_config.rules) {
                if (!r.enabled || !r.active) continue;
                
                // Ищем пин реле в списке actuators правила
                for (uint8_t actPin : r.actuators) {
                    if (actPin == ch.pin) {
                        shouldBeOn = true;
                        break;
                    }
                }
                if (shouldBeOn) break;
            }

            // Если есть активные правила для этого реле — управляем им
            bool hasActiveRules = false;
            for (const auto& r : g_config.rules) {
                 if (r.enabled && !r.actuators.empty()) {
                     for (uint8_t actPin : r.actuators) {
                         if (actPin == ch.pin) { hasActiveRules = true; break; }
                     }
                 }
                 if (hasActiveRules) break;
            }
            
            // Если у реле есть правила, мы применяем состояние. 
            // Если правил нет — не трогаем (оставляем ручное управление).
            if (hasActiveRules) {
                channels::setRelay(ch.pin, shouldBeOn);
            }
        }
    }
}