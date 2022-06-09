#include "db/ProxyEntity.hpp"

namespace NekoRay::fmt {
    QString SocksHttpBean::ToShareLink() {
        QUrl url;
        if (socks_http_type == type_HTTP) { // http
            if (stream->security == "tls") {
                url.setScheme("https");
            } else {
                url.setScheme("http");
            }
        } else {
            url.setScheme(QString("socks%1").arg(socks_http_type));
        }
        if (!username.isEmpty()) url.setUserName(username);
        if (!password.isEmpty()) url.setPassword(password);
        url.setHost(serverAddress);
        url.setPort(serverPort);
        return url.toString();
    }

    QString TrojanVLESSBean::ToShareLink() {
        QUrl url;
        url.setScheme(proxy_type == proxy_VLESS ? "vless" : "trojan");
        url.setUserName(password);
        url.setHost(serverAddress);
        url.setPort(serverPort);
        if (!name.isEmpty()) url.setFragment(UrlSafe_encode(name));
        if (!stream->sni.isEmpty()) url.setQuery(QUrlQuery{{"sni", stream->sni}});
        return url.toString();
    }

    QString ShadowSocksBean::ToShareLink() {
        QUrl url;
        url.setScheme("ss");
        auto username = method + ":" + password;
        url.setUserName(username.toUtf8().toBase64(QByteArray::Base64Option::Base64UrlEncoding));
        url.setHost(serverAddress);
        url.setPort(serverPort);
        if (!name.isEmpty()) url.setFragment(UrlSafe_encode(name));
        QUrlQuery q;
        if (!plugin.isEmpty()) q.addQueryItem("plugin", plugin);
        if (!q.isEmpty()) url.setQuery(q);
        return url.toString();
    }

    QString VMessBean::ToShareLink() {
        QJsonObject N{
                {"v",    2},
                {"ps",   name},
                {"add",  serverAddress},
                {"port", serverPort},
                {"id",   uuid},
                {"aid",  aid},
                {"net",  stream->network},
                {"host", stream->host},
                {"path", stream->path},
                {"type", stream->header_type},
                {"scy",  security},
                // TODO header type
                {"tls",  stream->security == "tls" ? "tls" : ""},
                {"sni",  stream->sni},
        };
        return "vmess://" + QJsonObject2QString(N, false).toUtf8().toBase64();
    }

}