#include "db/ProxyEntity.hpp"
#include "fmt/includes.h"

namespace NekoRay::fmt {
    void V2rayStreamSettings::BuildStreamSettingsSingBox(QJsonObject *outbound) {
        // https://sing-box.sagernet.org/configuration/shared/v2ray-transport

        if (network != "tcp") {
            QJsonObject transport{{"type", network}};
            if (network == "ws") {
                if (!path.isEmpty()) transport["path"] = path;
                if (!host.isEmpty()) transport["headers"] = QJsonObject{{"Host", host}};
                if (ws_early_data_length > 0) {
                    transport["max_early_data"] = ws_early_data_length;
                    transport["early_data_header_name"] = ws_early_data_name;
                }
            } else if (network == "http") {
                if (!path.isEmpty()) transport["path"] = path;
                if (!host.isEmpty()) transport["host"] = QList2QJsonArray(host.split(","));
            } else if (network == "grpc") {
                if (!path.isEmpty()) transport["service_name"] = path;
            }
            outbound->insert("transport", transport);
        }

        // 对应字段 tls
        if (security == "tls") {
            QJsonObject tls{{"enabled", true}};
            if (allow_insecure || dataStore->skip_cert) tls["insecure"] = true;
            if (!sni.trimmed().isEmpty()) tls["server_name"] = sni;
            if (!certificate.trimmed().isEmpty()) {
                tls["certificate"] = certificate.trimmed();
            }
            if (!alpn.trimmed().isEmpty()) {
                tls["alpn"] = QList2QJsonArray(alpn.split(","));
            }
            auto fp = utlsFingerprint.isEmpty() ? NekoRay::dataStore->utlsFingerprint : utlsFingerprint;
            if (!fp.isEmpty()) {
                tls["utls"] = QJsonObject{
                    {"enabled", true},
                    {"fingerprint", fp},
                };
            }
            if (!reality_pbk.trimmed().isEmpty()) {
                tls["reality"] = QJsonObject{
                    {"enabled", true},
                    {"public_key", reality_pbk},
                    {"short_id", reality_sid},
                };
            }
            outbound->insert("tls", tls);
        }

        if (!packet_encoding.isEmpty()) {
            auto pkt = packet_encoding;
            if (pkt == "packet") pkt = "packetaddr";
            outbound->insert("packet_encoding", pkt);
        }
    }

    CoreObjOutboundBuildResult SocksHttpBean::BuildCoreObjSingBox() {
        CoreObjOutboundBuildResult result;

        QJsonObject outbound;
        outbound["type"] = socks_http_type == type_HTTP ? "http" : "socks";
        if (socks_http_type == type_Socks4) outbound["version"] = "4";
        outbound["server"] = serverAddress;
        outbound["server_port"] = serverPort;

        if (!username.isEmpty() && !password.isEmpty()) {
            outbound["username"] = username;
            outbound["password"] = password;
        }

        stream->BuildStreamSettingsSingBox(&outbound);
        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult ShadowSocksBean::BuildCoreObjSingBox() {
        CoreObjOutboundBuildResult result;

        QJsonObject outbound{{"type", "shadowsocks"}};
        outbound["server"] = serverAddress;
        outbound["server_port"] = serverPort;
        outbound["method"] = method;
        outbound["password"] = password;

        if (!plugin.trimmed().isEmpty()) {
            outbound["plugin"] = SubStrBefore(plugin, ";");
            outbound["plugin_opts"] = SubStrAfter(plugin, ";");
        }

        stream->BuildStreamSettingsSingBox(&outbound);
        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult VMessBean::BuildCoreObjSingBox() {
        CoreObjOutboundBuildResult result;

        QJsonObject outbound{
            {"type", "vmess"},
            {"server", serverAddress},
            {"server_port", serverPort},
            {"uuid", uuid.trimmed()},
            {"alter_id", aid},
            {"security", security},
        };

        stream->BuildStreamSettingsSingBox(&outbound);
        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult TrojanVLESSBean::BuildCoreObjSingBox() {
        CoreObjOutboundBuildResult result;

        QJsonObject outbound{
            {"type", proxy_type == proxy_VLESS ? "vless" : "trojan"},
            {"server", serverAddress},
            {"server_port", serverPort},
        };

        QJsonObject settings;
        if (proxy_type == proxy_VLESS) {
            outbound["uuid"] = password.trimmed();
            outbound["flow"] = flow;
        } else {
            outbound["password"] = password;
        }

        stream->BuildStreamSettingsSingBox(&outbound);
        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult HysteriaBean::BuildCoreObjSingBox() {
        CoreObjOutboundBuildResult result;

        QJsonObject coreTlsObj{
            {"enabled", true},
            {"insecure", allowInsecure},
        };
        if (!alpn.trimmed().isEmpty()) coreTlsObj["alpn"] = QJsonArray{alpn};

        QJsonObject coreHysteriaObj{
            {"type", "hysteria"},
            {"server", serverAddress},
            {"server_port", serverPort},
            {"disable_mtu_discovery", disableMtuDiscovery},
            {"recv_window", streamReceiveWindow},
            {"recv_window_conn", connectionReceiveWindow},
            {"up_mbps", uploadMbps},
            {"down_mbps", downloadMbps},
            {"tls", coreTlsObj},
        };

        if (authPayloadType == hysteria_auth_base64) coreHysteriaObj["auth"] = authPayload;
        if (authPayloadType == hysteria_auth_string) coreHysteriaObj["auth_str"] = authPayload;

        result.outbound = coreHysteriaObj;
        return result;
    }

    CoreObjOutboundBuildResult CustomBean::BuildCoreObjSingBox() {
        CoreObjOutboundBuildResult result;

        if (core == "internal") {
            result.outbound = QString2QJsonObject(config_simple);
        }

        return result;
    }
} // namespace NekoRay::fmt
