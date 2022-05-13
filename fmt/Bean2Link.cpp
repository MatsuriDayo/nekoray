#include "db/ProxyEntity.hpp"

namespace NekoRay::fmt {
    QString SocksBean::ToShareLink() {
        QUrl url;
        url.setScheme(QString("socks%1").arg(socksVersion));
        if (!username.isEmpty()) url.setUserName(username);
        if (!password.isEmpty()) url.setPassword(password);
        url.setHost(serverAddress);
        url.setPort(serverPort);
        return url.toString();
    }

    QString TrojanBean::ToShareLink() {
        QUrl url;
        url.setScheme("trojan");
        url.setUserName(password);
        url.setHost(serverAddress);
        url.setPort(serverPort);
        if (!name.isEmpty()) url.setFragment(name); // TODO url safe
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
        if (!name.isEmpty()) url.setFragment(name); // TODO url safe
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