#include "db/ProxyEntity.hpp"

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
    ExternalBuildResult NaiveBean::BuildExternal(int mapping_port, int socks_port) {
        ExternalBuildResult result{dataStore->extraCore->Get("naive")};
        if (result.program.isEmpty()) {
            result.error = QObject::tr("Core not found: %1").arg(DisplayType());
            return result;
        }

        auto _serverAddress = sni.isEmpty() ? serverAddress : sni;

        result.arguments += "--log";
        result.arguments += "--listen=socks://127.0.0.1:" + Int2String(socks_port);
        result.arguments += "--proxy=" + protocol + "://" +
                            username + ":" + password + "@" +
                            _serverAddress + ":" + Int2String(mapping_port);
        result.arguments += "--host-resolver-rules=MAP " + _serverAddress + " 127.0.0.1";
        if (insecure_concurrency > 0) result.arguments += "--insecure-concurrency=" + Int2String(insecure_concurrency);
        if (!extra_headers.isEmpty()) result.arguments += "--extra-headers=" + extra_headers;
        if (!certificate.isEmpty()) {
            WriteTempFile("naive.crt", certificate.toUtf8());
            result.env += "SSL_CERT_FILE=" + TempFile;
        }

        return result;
    }

    ExternalBuildResult CustomBean::BuildExternal(int mapping_port, int socks_port) {
        ExternalBuildResult result{dataStore->extraCore->Get(core)};
        if (result.program.isEmpty()) {
            result.error = QObject::tr("Core not found: %1").arg(DisplayType());
            return result;
        }

        result.arguments = command; // TODO ?

        if (!config_simple.trimmed().isEmpty()) {
            auto config = config_simple;
            config = config.replace("%mapping_port%", Int2String(mapping_port));
            config = config.replace("%socks_port%", Int2String(socks_port));

            WriteTempFile("custom_cfg.tmp", config.toUtf8());
            for (int i = 0; i < result.arguments.count(); i++) {
                result.arguments[i] = result.arguments[i].replace("%config%", TempFile);
            }
        }

        return result;
    }
}