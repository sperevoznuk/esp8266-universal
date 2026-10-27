#pragma once
#include <Arduino.h>
#include <math.h>

// Округление float до 2 знаков после запятой — для строкового вывода (ArduinoJson)
inline String fmt2(float val) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%.2f", val);
    return String(buf);
}

// Округление float до 2 знаков — возвращает float (для числовых полей)
inline float round2(float val) {
    return roundf(val * 100.0f) / 100.0f;
}

inline void logf(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    Serial.print('[');
    Serial.print(millis());
    Serial.print("] ");
    Serial.println(buf);
}

inline String escapeHtml(const String& s) {
    String out = s;
    out.replace("&", "&amp;");
    out.replace("<", "&lt;");
    out.replace(">", "&gt;");
    out.replace("\"", "&quot;");
    return out;
}
