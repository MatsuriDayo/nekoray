#include "db/ProxyEntity.hpp"
#include "fmt/includes.h"

#include <QUrlQuery>

namespace NekoGui_fmt {

#define DECODE_V2RAY_N_1                                                                                                        \
    QString linkN = DecodeB64IfValid(SubStrBefore(SubStrAfter(link, "://"), "#"), QByteArray::Base64Option::Base64UrlEncoding); \
    if (linkN.isEmpty()) return false;                                                                                          \
    auto hasRemarks = link.contains("#");                                                                                       \
    if (hasRemarks) linkN += "#" + SubStrAfter(link, "#");                                                                      \
    auto url = QUrl("https://" + linkN);

    bool SocksHttpBean::TryParseLink(const QString &link) {
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

        // v2rayN fmt
        if (password.isEmpty() && !username.isEmpty()) {
            QString n = DecodeB64IfValid(username);
            if (!n.isEmpty()) {
                username = SubStrBefore(n, ":");
                password = SubStrAfter(n, ":");
            }
        }

        stream->security = GetQueryValue(query, "security", "");
        stream->sni = GetQueryValue(query, "sni");
        if (link.startsWith("https")) stream->security = "tls";

        return !serverAddress.isEmpty();
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

        // security

        auto type =  GetQueryValue(query, "type", "tcp");
        if (type == "h2") {
            type = "http";
        }
        stream->network = type;

        if (proxy_type == proxy_Trojan) {
            stream->security = GetQueryValue(query, "security", "tls").replace("reality", "tls").replace("none", "");
        } else {
            stream->security = GetQueryValue(query, "security", "").replace("reality", "tls").replace("none", "");
        }
        auto sni1 = GetQueryValue(query, "sni");
        auto sni2 = GetQueryValue(query, "peer");
        if (!sni1.isEmpty()) stream->sni = sni1;
        if (!sni2.isEmpty()) stream->sni = sni2;
        stream->alpn = GetQueryValue(query, "alpn");
        if (!query.queryItemValue("allowInsecure").isEmpty()) stream->allow_insecure = true;
        stream->reality_pbk = GetQueryValue(query, "pbk", "");
        stream->reality_sid = GetQueryValue(query, "sid", "");
        stream->reality_spx = GetQueryValue(query, "spx", "");
        stream->utlsFingerprint = GetQueryValue(query, "fp", "");
        if (stream->utlsFingerprint.isEmpty()) {
            stream->utlsFingerprint = NekoGui::dataStore->utlsFingerprint;
        }

        // type
        if (stream->network == "ws") {
            stream->path = GetQueryValue(query, "path", "");
            stream->host = GetQueryValue(query, "host", "");
        } else if (stream->network == "http") {
            stream->path = GetQueryValue(query, "path", "");
            stream->host = GetQueryValue(query, "host", "").replace("|", ",");
        } else if (stream->network == "httpupgrade") {
            stream->path = GetQueryValue(query, "path", "");
            stream->host = GetQueryValue(query, "host", "");
        } else if (stream->network == "grpc") {
            stream->path = GetQueryValue(query, "serviceName", "");
        } else if (stream->network == "tcp") {
            if (GetQueryValue(query, "headerType") == "http") {
                stream->header_type = "http";
                stream->host = GetQueryValue(query, "host", "");
                stream->path = GetQueryValue(query, "path", "");
            }
        }

        // protocol
        if (proxy_type == proxy_VLESS) {
            flow = GetQueryValue(query, "flow", "");
        }

        return !(password.isEmpty() || serverAddress.isEmpty());
    }

    bool ShadowSocksBean::TryParseLink(const QString &link) {
        if (SubStrBefore(link, "#").contains("@")) {
            // SS
            auto url = QUrl(link);
            if (!url.isValid()) return false;

            name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            serverPort = url.port();

            if (url.password().isEmpty()) {
                // traditional format
                auto method_password = DecodeB64IfValid(url.userName(), QByteArray::Base64Option::Base64UrlEncoding);
                if (method_password.isEmpty()) return false;
                method = SubStrBefore(method_password, ":");
                password = SubStrAfter(method_password, ":");
            } else {
                // 2022 format
                method = url.userName();
                password = url.password();
            }

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
        return !(serverAddress.isEmpty() || method.isEmpty() || password.isEmpty());
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
            auto net = objN["net"].toString();
            if (!net.isEmpty()) {
                if (net == "h2") {
                    net = "http";
                }
                stream->network = net;
            }
            auto scy = objN["scy"].toString();
            if (!scy.isEmpty()) security = scy;
            // TLS (XTLS?)
            stream->security = objN["tls"].toString();
            // TODO quic & kcp
            return true;
        } else {
            // https://github.com/XTLS/Xray-core/discussions/716
            auto url = QUrl(link);
            if (!url.isValid()) return false;
            auto query = GetQuery(url);

            name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            serverPort = url.port();
            uuid = url.userName();
            if (serverPort == -1) serverPort = 443;

            aid = 0; // “此分享标准仅针对 VMess AEAD 和 VLESS。”
            security = GetQueryValue(query, "encryption", "auto");

            // security
            auto type = GetQueryValue(query, "type", "tcp");
            if (type == "h2") {
                type = "http";
            }
            stream->network = type;
            stream->security = GetQueryValue(query, "security", "tls").replace("reality", "tls");
            auto sni1 = GetQueryValue(query, "sni");
            auto sni2 = GetQueryValue(query, "peer");
            if (!sni1.isEmpty()) stream->sni = sni1;
            if (!sni2.isEmpty()) stream->sni = sni2;
            if (!query.queryItemValue("allowInsecure").isEmpty()) stream->allow_insecure = true;
            stream->reality_pbk = GetQueryValue(query, "pbk", "");
            stream->reality_sid = GetQueryValue(query, "sid", "");
            stream->reality_spx = GetQueryValue(query, "spx", "");
            stream->utlsFingerprint = GetQueryValue(query, "fp", "");
            if (stream->utlsFingerprint.isEmpty()) {
                stream->utlsFingerprint = NekoGui::dataStore->utlsFingerprint;
            }

            // type
            if (stream->network == "ws") {
                stream->path = GetQueryValue(query, "path", "");
                stream->host = GetQueryValue(query, "host", "");
            } else if (stream->network == "http") {
                stream->path = GetQueryValue(query, "path", "");
                stream->host = GetQueryValue(query, "host", "").replace("|", ",");
            } else if (stream->network == "httpupgrade") {
                stream->path = GetQueryValue(query, "path", "");
                stream->host = GetQueryValue(query, "host", "");
            } else if (stream->network == "grpc") {
                stream->path = GetQueryValue(query, "serviceName", "");
            } else if (stream->network == "tcp") {
                if (GetQueryValue(query, "headerType") == "http") {
                    stream->header_type = "http";
                    stream->path = GetQueryValue(query, "path", "");
                    stream->host = GetQueryValue(query, "host", "");
                }
            }
            return !(uuid.isEmpty() || serverAddress.isEmpty());
        }

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

        return !(username.isEmpty() || password.isEmpty() || serverAddress.isEmpty());
    }

    bool QUICBean::TryParseLink(const QString &link) {
        auto url = QUrl(link);
        auto query = QUrlQuery(url.query());
        if (url.host().isEmpty() || url.port() == -1) return false;

        if (url.scheme() == "hysteria") {
            // https://hysteria.network/docs/uri-scheme/
            if (!query.hasQueryItem("upmbps") || !query.hasQueryItem("downmbps")) return false;

            name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            serverPort = url.port();
            hopPort = query.queryItemValue("mport");
            obfsPassword = query.queryItemValue("obfsParam");
            allowInsecure = QStringList{"1", "true"}.contains(query.queryItemValue("insecure"));
            uploadMbps = query.queryItemValue("upmbps").toInt();
            downloadMbps = query.queryItemValue("downmbps").toInt();

            auto protocolStr = (query.hasQueryItem("protocol") ? query.queryItemValue("protocol") : "udp").toLower();
            if (protocolStr == "faketcp") {
                hyProtocol = NekoGui_fmt::QUICBean::hysteria_protocol_facktcp;
            } else if (protocolStr.startsWith("wechat")) {
                hyProtocol = NekoGui_fmt::QUICBean::hysteria_protocol_wechat_video;
            }

            if (query.hasQueryItem("auth")) {
                authPayload = query.queryItemValue("auth");
                authPayloadType = NekoGui_fmt::QUICBean::hysteria_auth_string;
            }

            alpn = query.queryItemValue("alpn");
            sni = FIRST_OR_SECOND(query.queryItemValue("peer"), query.queryItemValue("sni"));

            connectionReceiveWindow = query.queryItemValue("recv_window").toInt();
            streamReceiveWindow = query.queryItemValue("recv_window_conn").toInt();
        } else if (url.scheme() == "tuic") {
            // by daeuniverse
            // https://github.com/daeuniverse/dae/discussions/182

            name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            if (serverPort == -1) serverPort = 443;
            serverPort = url.port();

            uuid = url.userName();
            password = url.password();

            congestionControl = query.queryItemValue("congestion_control");
            alpn = query.queryItemValue("alpn");
            sni = query.queryItemValue("sni");
            udpRelayMode = query.queryItemValue("udp_relay_mode");
            allowInsecure = query.queryItemValue("allow_insecure") == "1";
            disableSni = query.queryItemValue("disable_sni") == "1";
        } else if (QStringList{"hy2", "hysteria2"}.contains(url.scheme())) {
            name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            serverPort = url.port();
            hopPort = query.queryItemValue("mport");
            obfsPassword = query.queryItemValue("obfs-password");
            allowInsecure = QStringList{"1", "true"}.contains(query.queryItemValue("insecure"));

            if (url.password().isEmpty()) {
                password = url.userName();
            } else {
                password = url.userName() + ":" + url.password();
            }

            sni = query.queryItemValue("sni");
        }

        return true;
    }

} // namespace NekoGui_fmt