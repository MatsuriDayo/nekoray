#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <functional>

namespace Qv2ray::common::network {
    struct HTTPResponse {
        QString error;
        QByteArray data;
        QList<QPair<QByteArray, QByteArray>> header;
    };

    class NetworkRequestHelper : QObject {
    Q_OBJECT

        explicit NetworkRequestHelper(QObject *parent) : QObject(parent) {};

        ~NetworkRequestHelper() {};

    public:
        static void AsyncHttpGet(const QString &url, std::function<void(const QByteArray &)> funcPtr);

        static HTTPResponse HttpGet(const QUrl &url);

        static QString GetHeader(const QList<QPair<QByteArray, QByteArray>>& header, const QString& name);

    private:
        static void setAccessManagerAttributes(QNetworkRequest &request, QNetworkAccessManager &accessManager);

        static void setHeader(QNetworkRequest &request, const QByteArray &key, const QByteArray &value);
    };
} // namespace Qv2ray::common::network

using namespace Qv2ray::common::network;
