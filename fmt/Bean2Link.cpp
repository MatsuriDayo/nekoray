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
        if (!stream->sni.isEmpty()) url.setQuery(QUrlQuery{{"sni", stream->sni}});
        return url.toString();
    }
}