#include "db/ProxyEntity.hpp"

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
            if (!host.isEmpty()) h2["host"] = QJsonArray{host};
            streamSettings["httpSettings"] = h2;
        } else if (network == "grpc") {
            QJsonObject grpc;
            if (!path.isEmpty()) grpc["serviceName"] = path;
            streamSettings["grpcSettings"] = grpc;
        }

        if (security == "tls") {
            QJsonObject tls;
            if (!sni.isEmpty()) tls["serverName"] = sni;
            if (allow_insecure) tls["allowInsecure"] = true;
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
        outbound["settings"] = settings;
        outbound["streamSettings"] = stream->BuildStreamSettings();

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

        outbound["settings"] = settings;
        outbound["streamSettings"] = stream->BuildStreamSettings();

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

        outbound["settings"] = settings;
        outbound["streamSettings"] = stream->BuildStreamSettings();

        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult TrojanBean::BuildCoreObj() {
        CoreObjOutboundBuildResult result;
        QJsonObject outbound{
                {"protocol", "trojan"},
        };

        QJsonObject settings{
                {"servers", QJsonArray{
                        QJsonObject{
                                {"address",  serverAddress},
                                {"port",     serverPort},
                                {"password", password},
                        }
                }}
        };

        outbound["settings"] = settings;
        auto streamSettings = stream->BuildStreamSettings();
        auto sec = streamSettings["security"].toString();
        if (sec.isEmpty() || sec == "none") {
            result.error = QObject::tr("reject clear text trojan");
        }
        outbound["streamSettings"] = streamSettings;

        result.outbound = outbound;
        return result;
    }
}