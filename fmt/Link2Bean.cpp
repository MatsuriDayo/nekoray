#include <QUrlQuery>
#include "db/ProxyEntity.hpp"

namespace NekoRay::fmt {

#define DECODE_V2RAY_N_1 auto linkN = DecodeB64IfValid(SubStrBefore(SubStrAfter(link, "://"), "#"), QByteArray::Base64Option::Base64UrlEncoding); \
    if (linkN.isEmpty()) return false; \
    auto hasRemarks = link.contains("#"); \
    if (hasRemarks) linkN += "#" + SubStrAfter(link, "#"); \
    auto url = QUrl("https://" + linkN);

    bool SocksBean::TryParseLink(const QString &link) {
        if (!SubStrAfter(link, "://").contains(":")) {
            // v2rayN shit format
            DECODE_V2RAY_N_1

            if (hasRemarks) name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            serverPort = url.port();
            username = url.userName();
            password = url.password();
        } else {
            auto url = QUrl(link);
            if (!url.isValid()) return false;
            auto query = GetQuery(url);

            if (link.startsWith("socks4")) socksVersion = 4;
            serverAddress = url.host();
            serverPort = url.port();
            username = url.userName();
            password = url.password();
            stream->security = GetQueryValue(query, "security", "") == "true" ? "tls" : "none";
            stream->sni = GetQueryValue(query, "sni");
        }
        return true;
    }

    bool TrojanBean::TryParseLink(const QString &link) {
        auto url = QUrl(link);
        if (!url.isValid()) return false;
        auto query = GetQuery(url);

        name = url.fragment(QUrl::FullyDecoded);
        serverAddress = url.host();
        serverPort = url.port();
        password = url.userName();

        stream->security = GetQueryValue(query, "security", "tls");
        auto sni1 = GetQueryValue(query, "sni");
        auto sni2 = GetQueryValue(query, "peer");
        if (!sni1.isEmpty()) stream->sni = sni1;
        if (!sni2.isEmpty()) stream->sni = sni2;
        if (!query.queryItemValue("allowInsecure").isEmpty()) stream->allow_insecure = true;

        return true;
    }

    bool ShadowSocksBean::TryParseLink(const QString &link) {
        if (SubStrBefore(link, "#").contains("@")) {
            // SS
            auto url = QUrl(link);
            if (!url.isValid()) return false;

            name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            serverPort = url.port();
            auto method_password = DecodeB64IfValid(url.userName(), QByteArray::Base64Option::Base64UrlEncoding);
            if (method_password.isEmpty()) return false;
            method = SubStrBefore(method_password, ":");
            password = SubStrAfter(method_password, ":");
            auto query = GetQuery(url);
            plugin = query.queryItemValue("plugin");
        } else {
            // v2rayN
            DECODE_V2RAY_N_1

            if (hasRemarks) name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            serverPort = url.port();
            method = url.userName();
            password = url.password();
        }
        return true;
    }

    bool VMessBean::TryParseLink(const QString &link) {
        // V2RayN Format
        auto linkN = DecodeB64IfValid(SubStrAfter(link, "vmess://"));
        if (!linkN.isEmpty()) {
            auto objN = QString2QJsonObject(linkN);
            if (objN.isEmpty()) return false;
            // REQUIRED
            uuid = objN["id"].toString();
            serverAddress = objN["add"].toString();
            serverPort = objN["port"].toVariant().toInt();
            // OPTIONAL
            name = objN["ps"].toString();
            aid = objN["aid"].toInt();
            stream->host = objN["host"].toString();
            stream->path = objN["path"].toString();
            stream->sni = objN["sni"].toString();
            auto net = objN["net"].toString().replace("http", "h2");
            if (!net.isEmpty()) stream->network = net;
            auto scy = objN["scy"].toString();
            if (!scy.isEmpty()) security = scy;
            // TLS (XTLS?)
            if (!objN["tls"].toString().isEmpty()) stream->security = "tls";
            // TODO quic & kcp
            return true;
        }

        // Std Format
        return false;
    }

}