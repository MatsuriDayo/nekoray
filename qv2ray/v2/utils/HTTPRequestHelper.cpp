#include "HTTPRequestHelper.hpp"

#include <QByteArray>
#include <QNetworkProxy>
#include <QEventLoop>
#include <QMetaEnum>

#include "main/NekoRay.hpp"

#include "qv2ray/wrapper.hpp"

namespace Qv2ray::common::network {

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
            accessManager.setProxy(p);
            if (NekoRay::dataStore->started_id < 0) {
                return NekoHTTPResponse{QObject::tr("Request with proxy but no profile started.")};
            }
        }
        if (accessManager.proxy().type() == QNetworkProxy::Socks5Proxy) {
            DEBUG("Adding HostNameLookupCapability to proxy.");
            auto cap = accessManager.proxy().capabilities();
            accessManager.proxy().setCapabilities(cap | QNetworkProxy::HostNameLookupCapability);
        }
        // Set attribute
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, NekoRay::dataStore->user_agent);
        auto _reply = accessManager.get(request);
        //
        {
            QEventLoop loop;
            QObject::connect(&accessManager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
            loop.exec();
        }
        //
        return NekoHTTPResponse{_reply->error() == QNetworkReply::NetworkError::NoError ? "" : _reply->errorString(),
                                _reply->readAll(), _reply->rawHeaderPairs()};
    }

    QString NetworkRequestHelper::GetHeader(const QList<QPair<QByteArray, QByteArray>> &header, const QString &name) {
        for (const auto &p: header) {
            if (QString(p.first).toLower() == name.toLower()) return p.second;
        }
        return "";
    }

} // namespace Qv2ray::common::network
