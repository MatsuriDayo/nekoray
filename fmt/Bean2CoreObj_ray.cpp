#include "db/ProxyEntity.hpp"
#include "fmt/includes.h"

#define MAKE_SETTINGS_STREAM_SETTINGS                                                             \
    if (!stream->packet_encoding.isEmpty()) settings["packetEncoding"] = stream->packet_encoding; \
    outbound["settings"] = settings;                                                              \
    auto streamSettings = stream->BuildStreamSettingsV2Ray();                                     \
    outbound["streamSettings"] = streamSettings;

namespace NekoRay::fmt {
    QJsonObject V2rayStreamSettings::BuildStreamSettingsV2Ray() {
        QJsonObject streamSettings{{"network", network}};

        if (network == "ws") {
            QJsonObject ws;
            if (!host.isEmpty()) ws["headers"] = QJsonObject{{"Host", host}};
            // ws path & ed
            auto pathWithoutEd = SubStrBefore(path, "?ed=");
            if (!pathWithoutEd.isEmpty()) ws["path"] = pathWithoutEd;
            if (pathWithoutEd != path) {
                auto ed = SubStrAfter(path, "?ed=").toInt();
                if (ed > 0) {
                    ws["maxEarlyData"] = ed;
                    ws["earlyDataHeaderName"] = "Sec-WebSocket-Protocol";
                }
            }
            if (ws_early_data_length > 0) {
                ws["maxEarlyData"] = ws_early_data_length;
                ws["earlyDataHeaderName"] = ws_early_data_name;
            }
            streamSettings["wsSettings"] = ws;
        } else if (network == "http") {
            QJsonObject http;
            if (!path.isEmpty()) http["path"] = path;
            if (!host.isEmpty()) http["host"] = QList2QJsonArray(host.split(","));
            streamSettings["httpSettings"] = http;
        } else if (network == "grpc") {
            QJsonObject grpc;
            if (!path.isEmpty()) grpc["serviceName"] = path;
            streamSettings["grpcSettings"] = grpc;
        } else if (network == "quic") {
            QJsonObject quic;
            if (!header_type.isEmpty()) quic["header"] = QJsonObject{{"type", header_type}};
            if (!path.isEmpty()) quic["key"] = path;
            if (!host.isEmpty()) quic["security"] = host;
            streamSettings["quicSettings"] = quic;
        } else if (network == "tcp" && !header_type.isEmpty()) {
            QJsonObject header{{"type", header_type}};
            if (header_type == "http") {
                header["request"] = QJsonObject{
                    {"path", QList2QJsonArray(path.split(","))},
                    {"headers", QJsonObject{{"Host", QList2QJsonArray(host.split(","))}}},
                };
            }
            streamSettings["tcpSettings"] = QJsonObject{{"header", header}};
        }

        if (security == "tls") {
            auto fp = utlsFingerprint.isEmpty() ? NekoRay::dataStore->utlsFingerprint : utlsFingerprint;
            bool v5_utls = !fp.isEmpty();
            QJsonObject tls;
            if (allow_insecure || dataStore->skip_cert) tls["allowInsecure"] = true;
            if (!sni.trimmed().isEmpty()) tls["serverName"] = sni;
            if (!certificate.trimmed().isEmpty()) {
                tls["disableSystemRoot"] = true;
                tls["certificates"] = QJsonArray{
                    QJsonObject{
                        {"usage", v5_utls ? "ENCIPHERMENT" : "verify"},
                        {"certificate", QList2QJsonArray(SplitLines(certificate.trimmed()))},
                    },
                };
            }
            if (!alpn.trimmed().isEmpty()) {
                tls[v5_utls ? "nextProtocol" : "alpn"] = QList2QJsonArray(alpn.split(","));
            }
            if (v5_utls) {
                streamSettings["utlsSettings"] = QJsonObject{
                    {"imitate", fp},
                    {"tlsConfig", tls},
                };
                streamSettings["security"] = "utls";
            } else {
                streamSettings["tlsSettings"] = tls;
                streamSettings["security"] = "tls";
            }
        }

        return streamSettings;
    }

    CoreObjOutboundBuildResult SocksHttpBean::BuildCoreObjV2Ray() {
        CoreObjOutboundBuildResult result;

        QJsonObject outbound;
        outbound["protocol"] = socks_http_type == type_HTTP ? "http" : "socks";

        QJsonObject settings;
        QJsonArray servers;
        QJsonObject server;

        server["address"] = serverAddress;
        server["port"] = serverPort;

        QJsonArray users;
        QJsonObject user;
        user["user"] = username;
        user["pass"] = password;
        users.push_back(user);
        if (!username.isEmpty() && !password.isEmpty()) server["users"] = users;

        servers.push_back(server);
        settings["servers"] = servers;

        MAKE_SETTINGS_STREAM_SETTINGS

        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult ShadowSocksBean::BuildCoreObjV2Ray() {
        CoreObjOutboundBuildResult result;

        QJsonObject outbound{{"protocol", "shadowsocks"}};

        QJsonObject settings;
        QJsonArray servers;
        QJsonObject server;

        server["address"] = serverAddress;
        server["port"] = serverPort;
        server["method"] = method;
        server["password"] = password;

        servers.push_back(server);
        settings["servers"] = servers;

        if (!plugin.trimmed().isEmpty()) {
            settings["plugin"] = SubStrBefore(plugin, ";");
            settings["pluginOpts"] = SubStrAfter(plugin, ";");
        }

        MAKE_SETTINGS_STREAM_SETTINGS

        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult VMessBean::BuildCoreObjV2Ray() {
        CoreObjOutboundBuildResult result;
        QJsonObject outbound{{"protocol", "vmess"}};

        QJsonObject settings{
            {"vnext", QJsonArray{
                          QJsonObject{
                              {"address", serverAddress},
                              {"port", serverPort},
                              {"users", QJsonArray{
                                            QJsonObject{
                                                {"id", uuid.trimmed()},
                                                {"alterId", aid},
                                                {"security", security},
                                            }}},
                          }}}};

        MAKE_SETTINGS_STREAM_SETTINGS

        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult TrojanVLESSBean::BuildCoreObjV2Ray() {
        CoreObjOutboundBuildResult result;
        QJsonObject outbound{
            {"protocol", proxy_type == proxy_VLESS ? "vless" : "trojan"},
        };

        QJsonObject settings;
        if (proxy_type == proxy_VLESS) {
            settings = QJsonObject{
                {"vnext", QJsonArray{
                              QJsonObject{
                                  {"address", serverAddress},
                                  {"port", serverPort},
                                  {"users", QJsonArray{
                                                QJsonObject{
                                                    {"id", password.trimmed()},
                                                    {"encryption", "none"},
                                                }}},
                              }}}};
        } else {
            settings = QJsonObject{
                {"servers", QJsonArray{
                                QJsonObject{
                                    {"address", serverAddress},
                                    {"port", serverPort},
                                    {"password", password},
                                }}}};
        }

        MAKE_SETTINGS_STREAM_SETTINGS

        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult CustomBean::BuildCoreObjV2Ray() {
        CoreObjOutboundBuildResult result;

        if (core == "internal") {
            result.outbound = QString2QJsonObject(config_simple);
        }

        return result;
    }
} // namespace NekoRay::fmt