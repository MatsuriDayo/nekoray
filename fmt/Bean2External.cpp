#include "db/ProxyEntity.hpp"
#include "fmt/includes.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

#define WriteTempFile(fn, data)                                   \
    QDir dir;                                                     \
    if (!dir.exists("temp")) dir.mkdir("temp");                   \
    QFile f(QString("temp/") + fn);                               \
    bool ok = f.open(QIODevice::WriteOnly | QIODevice::Truncate); \
    if (ok) {                                                     \
        f.write(data);                                            \
    } else {                                                      \
        result.error = f.errorString();                           \
    }                                                             \
    f.close();                                                    \
    auto TempFile = QFileInfo(f).absoluteFilePath();

namespace NekoGui_fmt {
    // -1: Cannot use this config
    // 0: Internal
    // 1: Mapping External
    // 2: Direct External

    int NaiveBean::NeedExternal(bool isFirstProfile) {
        if (isFirstProfile) {
            if (NekoGui::dataStore->spmode_vpn) {
                return 1;
            }
            return 2;
        }
        return 1;
    }

    int QUICBean::NeedExternal(bool isFirstProfile) {
        auto extCore = [=] {
            if (isFirstProfile) {
                if (NekoGui::dataStore->spmode_vpn && hopPort.trimmed().isEmpty()) {
                    return 1;
                }
                return 2;
            } else {
                if (!hopPort.trimmed().isEmpty()) {
                    return -1;
                }
            }
            return 1;
        };

        if (!forceExternal && (proxy_type == proxy_TUIC || hopPort.trimmed().isEmpty())) {
            // sing-box support
            return 0;
        } else {
            // external core support
            return extCore();
        }
    }

    int CustomBean::NeedExternal(bool isFirstProfile) {
        if (core == "internal" || core == "internal-full") return 0;
        return 1;
    }

    ExternalBuildResult NaiveBean::BuildExternal(int mapping_port, int socks_port, int external_stat) {
        ExternalBuildResult result{NekoGui::dataStore->extraCore->Get("naive")};

        auto is_direct = external_stat == 2;
        auto domain_address = sni.isEmpty() ? serverAddress : sni;
        auto connect_address = is_direct ? serverAddress : "127.0.0.1";
        auto connect_port = is_direct ? serverPort : mapping_port;
        domain_address = WrapIPV6Host(domain_address);
        connect_address = WrapIPV6Host(connect_address);

        auto proxy_url = QUrl();
        proxy_url.setScheme(protocol);
        proxy_url.setUserName(username);
        proxy_url.setPassword(password);
        proxy_url.setPort(connect_port);
        proxy_url.setHost(domain_address);

        if (!disable_log) result.arguments += "--log";
        result.arguments += "--listen=socks://127.0.0.1:" + Int2String(socks_port);
        result.arguments += "--proxy=" + proxy_url.toString(QUrl::FullyEncoded);
        if (domain_address != connect_address)
            result.arguments += "--host-resolver-rules=MAP " + domain_address + " " + connect_address;
        if (insecure_concurrency > 0) result.arguments += "--insecure-concurrency=" + Int2String(insecure_concurrency);
        if (!extra_headers.trimmed().isEmpty()) result.arguments += "--extra-headers=" + extra_headers;
        if (!certificate.trimmed().isEmpty()) {
            WriteTempFile("naive_" + GetRandomString(10) + ".crt", certificate.toUtf8());
            result.env += "SSL_CERT_FILE=" + TempFile;
        }

        auto config_export = QStringList{result.program};
        config_export += result.arguments;
        result.config_export = QStringList2Command(config_export);

        return result;
    }

    ExternalBuildResult QUICBean::BuildExternal(int mapping_port, int socks_port, int external_stat) {
        if (proxy_type == proxy_TUIC) {
            ExternalBuildResult result{NekoGui::dataStore->extraCore->Get("tuic")};

            QJsonObject relay;

            relay["uuid"] = uuid;
            relay["password"] = password;
            relay["udp_relay_mode"] = udpRelayMode;
            relay["congestion_control"] = congestionControl;
            relay["zero_rtt_handshake"] = zeroRttHandshake;
            relay["disable_sni"] = disableSni;
            if (!heartbeat.trimmed().isEmpty()) relay["heartbeat"] = heartbeat;
            if (!alpn.trimmed().isEmpty()) relay["alpn"] = QList2QJsonArray(alpn.split(","));

            if (!caText.trimmed().isEmpty()) {
                WriteTempFile("tuic_" + GetRandomString(10) + ".crt", caText.toUtf8());
                QJsonArray certificate;
                certificate.append(TempFile);
                relay["certificates"] = certificate;
            }

            // The most confused part of TUIC......
            if (serverAddress == sni) {
                relay["server"] = serverAddress + ":" + Int2String(serverPort);
            } else {
                relay["server"] = sni + ":" + Int2String(serverPort);
                relay["ip"] = serverAddress;
            }

            QJsonObject local{
                {"server", "127.0.0.1:" + Int2String(socks_port)},
            };

            QJsonObject config{
                {"relay", relay},
                {"local", local},
            };

            //

            result.config_export = QJsonObject2QString(config, false);
            WriteTempFile("tuic_" + GetRandomString(10) + ".json", result.config_export.toUtf8());
            result.arguments = QStringList{"-c", TempFile};

            return result;
        } else if (proxy_type == proxy_Hysteria2) {
            ExternalBuildResult result{NekoGui::dataStore->extraCore->Get("hysteria2")};

            QJsonObject config;

            auto server = serverAddress;
            if (!hopPort.trimmed().isEmpty()) {
                server = WrapIPV6Host(server) + ":" + hopPort;
            } else {
                server = WrapIPV6Host(server) + ":" + Int2String(serverPort);
            }

            QJsonObject transport;
            transport["type"] = "udp";
            transport["udp"] = QJsonObject{
                {"hopInterval", QString::number(hopInterval) + "s"},
            };
            config["transport"] = transport;

            config["server"] = server;
            config["socks5"] = QJsonObject{
                {"listen", "127.0.0.1:" + Int2String(socks_port)},
                {"disableUDP", false},
            };
            config["auth"] = password;

            QJsonObject bandwidth;
            if (uploadMbps > 0) bandwidth["up"] = Int2String(uploadMbps) + " mbps";
            if (downloadMbps > 0) bandwidth["down"] = Int2String(downloadMbps) + " mbps";
            config["bandwidth"] = bandwidth;

            QJsonObject quic;
            if (streamReceiveWindow > 0) quic["initStreamReceiveWindow"] = streamReceiveWindow;
            if (connectionReceiveWindow > 0) quic["initConnReceiveWindow"] = connectionReceiveWindow;
            if (disableMtuDiscovery) quic["disablePathMTUDiscovery"] = true;
            config["quic"] = quic;

            config["fastOpen"] = true;
            config["lazy"] = true;

            if (!obfsPassword.isEmpty()) {
                QJsonObject obfs;
                obfs["type"] = "salamander";
                obfs["salamander"] = QJsonObject{
                    {"password", obfsPassword},
                };

                config["obfs"] = obfs;
            }

            QJsonObject tls;
            auto sniGen = sni;
            if (sni.isEmpty() && !IsIpAddress(serverAddress)) sniGen = serverAddress;
            tls["sni"] = sniGen;
            if (allowInsecure) tls["insecure"] = true;
            if (!caText.trimmed().isEmpty()) {
                WriteTempFile("hysteria2_" + GetRandomString(10) + ".crt", caText.toUtf8());
                QJsonArray certificate;
                certificate.append(TempFile);
                tls["certificates"] = certificate;
            }
            config["tls"] = tls;

            result.config_export = QJsonObject2QString(config, false);
            WriteTempFile("hysteria2_" + GetRandomString(10) + ".json", result.config_export.toUtf8());
            result.arguments = QStringList{"-c", TempFile};

            return result;
        }
        ExternalBuildResult e;
        e.error = "unknown type";
        return e;
    }

    ExternalBuildResult CustomBean::BuildExternal(int mapping_port, int socks_port, int external_stat) {
        ExternalBuildResult result{NekoGui::dataStore->extraCore->Get(core)};

        result.arguments = command; // TODO split?

        for (int i = 0; i < result.arguments.length(); i++) {
            auto arg = result.arguments[i];
            arg = arg.replace("%mapping_port%", Int2String(mapping_port));
            arg = arg.replace("%socks_port%", Int2String(socks_port));
            arg = arg.replace("%server_addr%", serverAddress);
            arg = arg.replace("%server_port%", Int2String(serverPort));
            result.arguments[i] = arg;
        }

        if (!config_simple.trimmed().isEmpty()) {
            auto config = config_simple;
            config = config.replace("%mapping_port%", Int2String(mapping_port));
            config = config.replace("%socks_port%", Int2String(socks_port));
            config = config.replace("%server_addr%", serverAddress);
            config = config.replace("%server_port%", Int2String(serverPort));

            // suffix
            QString suffix;
            if (!config_suffix.isEmpty()) {
                suffix = "." + config_suffix;
            } else if (!QString2QJsonObject(config).isEmpty()) {
                // trojan-go: unsupported config format: xxx.tmp. use .yaml or .json instead.
                suffix = ".json";
            }

            // write config
            WriteTempFile("custom_" + GetRandomString(10) + suffix, config.toUtf8());
            for (int i = 0; i < result.arguments.count(); i++) {
                result.arguments[i] = result.arguments[i].replace("%config%", TempFile);
            }

            result.config_export = config;
        }

        return result;
    }

} // namespace NekoGui_fmt
