#pragma once

// Qv2ray wrapper

#define LOG(...) Qv2ray::base::log_internal(__VA_ARGS__)
#define DEBUG(...) Qv2ray::base::log_internal(__VA_ARGS__)
namespace Qv2ray::base {
    template<typename... T>
    inline void log_internal(T... v) {}
}

#define JsonToString(a) QJsonObject2QString(a,false)
#define JsonFromString(a) QString2QJsonObject(a)
#define QvMessageBoxWarn(a, b, c) MessageBoxWarning(b,c)
