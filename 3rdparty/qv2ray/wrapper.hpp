#pragma once

// Qv2ray wrapper

#include <QJsonDocument>
#include <QDebug>

#define LOG(...) Qv2ray::base::log_internal(__VA_ARGS__)
#define DEBUG(...) Qv2ray::base::log_internal(__VA_ARGS__)
namespace Qv2ray {
    namespace base {
        template<typename... T>
        inline void log_internal(T... v) {}
    } // namespace base
} // namespace Qv2ray

#define JsonToString(a) QJsonObject2QString(a, false)
#define JsonFromString(a) QString2QJsonObject(a)
#define QvMessageBoxWarn(a, b, c) MessageBoxWarning(b, c)

inline QString VerifyJsonString(const QString &source) {
    QJsonParseError error{};
    QJsonDocument doc = QJsonDocument::fromJson(source.toUtf8(), &error);
    Q_UNUSED(doc)

    if (error.error == QJsonParseError::NoError) {
        return "";
    } else {
        // LOG("WARNING: Json parse returns: " + error.errorString());
        return error.errorString();
    }
}

#define RED(obj)                                 \
    {                                            \
        auto _temp = obj->palette();             \
        _temp.setColor(QPalette::Text, Qt::red); \
        obj->setPalette(_temp);                  \
    }

#define BLACK(obj) obj->setPalette(QWidget::palette());
