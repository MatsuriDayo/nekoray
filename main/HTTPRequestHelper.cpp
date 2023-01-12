#include "HTTPRequestHelper.hpp"

#include <QByteArray>
#include <QNetworkProxy>
#include <QEventLoop>
#include <QMetaEnum>

#include "main/NekoRay.hpp"

namespace NekoRay::network {

    NekoHTTPResponse NetworkRequestHelper::HttpGet(const QUrl &url) {
        QNetworkRequest request;
        QNetworkAccessManager accessManager;
        request.setUrl(url);
        // Set proxy
        if (NekoRay::dataStore->sub_use_proxy) {
            QNetworkProxy p;
            // Note: sing-box mixed socks5 protocol error
            p.setType(IS_NEKO_BOX ? QNetworkProxy::HttpProxy : QNetworkProxy::Socks5Proxy);
            p.setHostName("127.0.0.1");
            p.setPort(NekoRay::dataStore->inbound_socks_port);
            if (dataStore->inbound_auth->NeedAuth()) {
                p.setUser(dataStore->inbound_auth->username);
                p.setPassword(dataStore->inbound_auth->password);
            }
            accessManager.setProxy(p);
            if (NekoRay::dataStore->started_id < 0) {
                return NekoHTTPResponse{QObject::tr("Request with proxy but no profile started.")};
            }
        }
        if (accessManager.proxy().type() == QNetworkProxy::Socks5Proxy) {
            auto cap = accessManager.proxy().capabilities();
            accessManager.proxy().setCapabilities(cap | QNetworkProxy::HostNameLookupCapability);
        }
        // Set attribute
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, NekoRay::dataStore->user_agent);
        if (NekoRay::dataStore->sub_insecure) {
            QSslConfiguration c;
            c.setPeerVerifyMode(QSslSocket::PeerVerifyMode::VerifyNone);
            request.setSslConfiguration(c);
        }
        //
        auto _reply = accessManager.get(request);
        connect(_reply, &QNetworkReply::sslErrors, _reply, [](const QList<QSslError> &errors) {
            QStringList error_str;
            for (const auto &err: errors) {
                error_str << err.errorString();
            }
            MW_show_log(QString("SSL Errors: %1 %2").arg(error_str.join(","), NekoRay::dataStore->sub_insecure ? "(Ignored)" : ""));
        });
        //
        {
            QEventLoop loop;
            QObject::connect(&accessManager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
            loop.exec();
        }
        //
        auto result = NekoHTTPResponse{_reply->error() == QNetworkReply::NetworkError::NoError ? "" : _reply->errorString(),
                                       _reply->readAll(), _reply->rawHeaderPairs()};
        _reply->deleteLater();
        return result;
    }

    QString NetworkRequestHelper::GetHeader(const QList<QPair<QByteArray, QByteArray>> &header, const QString &name) {
        for (const auto &p: header) {
            if (QString(p.first).toLower() == name.toLower()) return p.second;
        }
        return "";
    }

} // namespace NekoRay::network
