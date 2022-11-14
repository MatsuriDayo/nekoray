#include "db/ProxyEntity.hpp"
#include "fmt/includes.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>

#define WriteTempFile(fn, data) \
QDir dir; \
if (!dir.exists("temp")) dir.mkdir("temp"); \
QFile f(QString("temp/") + fn); \
bool ok = f.open(QIODevice::WriteOnly | QIODevice::Truncate); \
if (ok) { \
f.write(data); \
} else { \
result.error = f.errorString(); \
} \
f.close(); \
auto TempFile = QFileInfo(f).absoluteFilePath();

namespace NekoRay::fmt {
    // 0: no external
    // 1: Mapping External
    // 2: Direct External

    int NaiveBean::NeedExternal(bool isFirstProfile, bool isVPN) {
        if (isFirstProfile && !isVPN) {
            return 2;
        }
        return 1;
    }

    int CustomBean::NeedExternal(bool isFirstProfile, bool isVPN) {
        if (core == "internal") return 0;
        if (IS_NEKO_BOX && core == "hysteria") return 0;
        if (core == "hysteria") {
            if (isFirstProfile && !isVPN) {
                return 2;
            }
        }
        return 1;
    }

    ExternalBuildResult NaiveBean::BuildExternal(int mapping_port, int socks_port, int external_stat) {
        ExternalBuildResult result{dataStore->extraCore->Get("naive")};

        auto is_direct = external_stat == 2;
        auto domain_address = sni.isEmpty() ? serverAddress : sni;
        auto connect_address = is_direct ? serverAddress : "127.0.0.1";
        auto connect_port = is_direct ? serverPort : mapping_port;
        domain_address = WrapIPV6Host(domain_address);
        connect_address = WrapIPV6Host(connect_address);

        result.arguments += "--log";
        result.arguments += "--listen=socks://127.0.0.1:" + Int2String(socks_port);
        result.arguments += "--proxy=" + protocol + "://" + username + ":" + password + "@" +
                            domain_address + ":" + Int2String(connect_port);
        if (domain_address != connect_address)
            result.arguments += "--host-resolver-rules=MAP " + domain_address + " " + connect_address;
        if (insecure_concurrency > 0) result.arguments += "--insecure-concurrency=" + Int2String(insecure_concurrency);
        if (!extra_headers.isEmpty()) result.arguments += "--extra-headers=" + extra_headers;
        if (!certificate.isEmpty()) {
            WriteTempFile("naive_" + GetRandomString(10) + ".crt", certificate.toUtf8());
            result.env += "SSL_CERT_FILE=" + TempFile;
        }

        auto config_export = QStringList{result.program};
        config_export += result.arguments;
        result.config_export = QStringList2Command(config_export);

        return result;
    }

    ExternalBuildResult CustomBean::BuildExternal(int mapping_port, int socks_port, int external_stat) {
        ExternalBuildResult result{dataStore->extraCore->Get(core)};

        result.arguments = command; // TODO split?

        for (int i = 0; i < result.arguments.length(); i++) {
            auto arg = result.arguments[i];
            if (arg.contains("%mapping_port%")) {
                arg = arg.replace("%mapping_port%", Int2String(mapping_port));
            } else if (arg.contains("%socks_port%")) {
                arg = arg.replace("%socks_port%", Int2String(socks_port));
            } else {
                continue;
            }
            result.arguments[i] = arg;
        }

        if (!config_simple.trimmed().isEmpty()) {
            auto config = config_simple;
            config = config.replace("%mapping_port%", Int2String(mapping_port));
            config = config.replace("%socks_port%", Int2String(socks_port));

            // trojan-go: unsupported config format: xxx.tmp. use .yaml or .json instead.
            auto suffix = ".tmp";
            if (!QString2QJsonObject(config).isEmpty()) {
                suffix = ".json";
            }

            // known core direct out
            if (external_stat == 2) {
                if (core == "hysteria") {
                    config = config.replace(QString("\"127.0.0.1:%1\"").arg(mapping_port),
                                            "\"" + DisplayAddress() + "\"");
                }
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
}