#include "db/ProxyEntity.hpp"
#include "fmt/includes.h"

#define MAKE_SETTINGS_STREAM_SETTINGS \
if (!stream->packet_encoding.isEmpty()) settings["packetEncoding"] = stream->packet_encoding; \
outbound["settings"] = settings; \
auto streamSettings = stream->BuildStreamSettings(); \
outbound["streamSettings"] = streamSettings;

namespace NekoRay::fmt {

    QJsonObject V2rayStreamSettings::BuildStreamSettings() {
        QJsonObject streamSettings{
                {"network",  network},
                {"security", security},
        };

        if (network == "ws") {
            QJsonObject ws;
            if (!path.isEmpty()) ws["path"] = path;
            if (!host.isEmpty()) ws["headers"] = QJsonObject{{"Host", host}};
            streamSettings["wsSettings"] = ws;
        } else if (network == "h2") {
            QJsonObject h2;
            if (!path.isEmpty()) h2["path"] = path;
            if (!host.isEmpty()) h2["host"] = QList2QJsonArray(host.split(","));
            streamSettings["httpSettings"] = h2;
        } else if (network == "grpc") {
            QJsonObject grpc;
            if (!path.isEmpty()) grpc["serviceName"] = path;
            streamSettings["grpcSettings"] = grpc;
        }

        if (security == "tls") {
            QJsonObject tls;
            if (!sni.isEmpty()) tls["serverName"] = sni;
            if (allow_insecure || dataStore->skip_cert) tls["allowInsecure"] = true;
            if (!certificate.isEmpty())
                tls["certificates"] = QJsonArray{
                        QJsonObject{
                                {"certificate", certificate},
                        },
                };
            streamSettings["tlsSettings"] = tls;
        }

        return streamSettings;
    }

    CoreObjOutboundBuildResult SocksHttpBean::BuildCoreObj() {
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

    CoreObjOutboundBuildResult ShadowSocksBean::BuildCoreObj() {
        CoreObjOutboundBuildResult result;

        QJsonObject outbound;
        outbound["protocol"] = "shadowsocks";

        QJsonObject settings;
        QJsonArray servers;
        QJsonObject server;

        server["address"] = serverAddress;
        server["port"] = serverPort;
        server["method"] = method;
        server["password"] = password;

        servers.push_back(server);
        settings["servers"] = servers;

        if (!plugin.isEmpty()) {
            settings["plugin"] = SubStrBefore(plugin, ";");
            settings["pluginOpts"] = SubStrAfter(plugin, ";");
        }

        MAKE_SETTINGS_STREAM_SETTINGS

        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult VMessBean::BuildCoreObj() {
        CoreObjOutboundBuildResult result;
        QJsonObject outbound{
                {"protocol", "vmess"},
        };

        QJsonObject settings{
                {"vnext", QJsonArray{
                        QJsonObject{
                                {"address", serverAddress},
                                {"port",    serverPort},
                                {"users",   QJsonArray{
                                        QJsonObject{
                                                {"id",       uuid},
                                                {"alterId",  aid},
                                                {"security", security},
                                        }
                                }},
                        }
                }}
        };

        MAKE_SETTINGS_STREAM_SETTINGS

        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult TrojanVLESSBean::BuildCoreObj() {
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
                                    {"port",    serverPort},
                                    {"users",   QJsonArray{
                                            QJsonObject{
                                                    {"id",         password},
                                                    {"encryption", "none"},
                                            }
                                    }},
                            }
                    }}
            };
        } else {
            settings = QJsonObject{
                    {"servers", QJsonArray{
                            QJsonObject{
                                    {"address",  serverAddress},
                                    {"port",     serverPort},
                                    {"password", password},
                            }
                    }}
            };
        }

        MAKE_SETTINGS_STREAM_SETTINGS

        result.outbound = outbound;
        return result;
    }
}