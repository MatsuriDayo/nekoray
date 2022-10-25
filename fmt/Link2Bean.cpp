#include "db/ProxyEntity.hpp"
#include "fmt/includes.h"

#include <QUrlQuery>

namespace NekoRay::fmt {

#define DECODE_V2RAY_N_1 auto linkN = DecodeB64IfValid(SubStrBefore(SubStrAfter(link, "://"), "#"), QByteArray::Base64Option::Base64UrlEncoding); \
    if (linkN.isEmpty()) return false; \
    auto hasRemarks = link.contains("#"); \
    if (hasRemarks) linkN += "#" + SubStrAfter(link, "#"); \
    auto url = QUrl("https://" + linkN);

    bool SocksHttpBean::TryParseLink(const QString &link) {
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

            if (link.startsWith("socks4")) socks_http_type = type_Socks4;
            if (link.startsWith("http")) socks_http_type = type_HTTP;
            name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            serverPort = url.port();
            username = url.userName();
            password = url.password();
            if (serverPort == -1) serverPort = socks_http_type == type_HTTP ? 443 : 1080;

            stream->security = GetQueryValue(query, "security", "") == "true" ? "tls" : "none";
            stream->sni = GetQueryValue(query, "sni");
        }
        return true;
    }

    bool TrojanVLESSBean::TryParseLink(const QString &link) {
        auto url = QUrl(link);
        if (!url.isValid()) return false;
        auto query = GetQuery(url);

        name = url.fragment(QUrl::FullyDecoded);
        serverAddress = url.host();
        serverPort = url.port();
        password = url.userName();
        if (serverPort == -1) serverPort = 443;

        stream->network = GetQueryValue(query, "type", "tcp").replace("http", "h2");
        stream->security = GetQueryValue(query, "security", "tls");
        auto sni1 = GetQueryValue(query, "sni");
        auto sni2 = GetQueryValue(query, "peer");
        if (!sni1.isEmpty()) stream->sni = sni1;
        if (!sni2.isEmpty()) stream->sni = sni2;
        if (!query.queryItemValue("allowInsecure").isEmpty()) stream->allow_insecure = true;

        // TODO header kcp quic
        if (stream->network == "ws") {
            stream->path = GetQueryValue(query, "path", "");
            stream->host = GetQueryValue(query, "host", "");
        } else if (stream->network == "h2") {
            stream->path = GetQueryValue(query, "path", "");
            stream->host = GetQueryValue(query, "host", "").replace("|", ",");
        } else if (stream->network == "grpc") {
            stream->path = GetQueryValue(query, "serviceName", "");
        }

        return !password.isEmpty();
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
            plugin = query.queryItemValue("plugin").replace("simple-obfs;", "obfs-local;");
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
            aid = objN["aid"].toVariant().toInt();
            stream->host = objN["host"].toString();
            stream->path = objN["path"].toString();
            stream->sni = objN["sni"].toString();
            stream->header_type = objN["type"].toString();
            auto net = objN["net"].toString().replace("http", "h2");
            if (!net.isEmpty()) stream->network = net;
            auto scy = objN["scy"].toString();
            if (!scy.isEmpty()) security = scy;
            // TLS (XTLS?)
            if (!objN["tls"].toString().isEmpty() && objN["tls"].toString().toLower() != 'none') stream->security = "tls";
            if (stream->security == "tls" && IsIpAddress(serverAddress) &&
                (!stream->host.isEmpty()) && stream->sni.isEmpty()) {
                // v2rayN config builder generate sni like this, so set sni here for their format.
                stream->sni = stream->host;
            }
            // TODO quic & kcp
            return true;
        }

        // Std Format
        return false;
    }

    bool NaiveBean::TryParseLink(const QString &link) {
        auto url = QUrl(link);
        if (!url.isValid()) return false;

        protocol = url.scheme().replace("naive+", "");
        if (protocol != "https" && protocol != "quic") return false;

        name = url.fragment(QUrl::FullyDecoded);
        serverAddress = url.host();
        serverPort = url.port();
        username = url.userName();
        password = url.password();

        return !(username.isEmpty() || password.isEmpty());
    }

}
