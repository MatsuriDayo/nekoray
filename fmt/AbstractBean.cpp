#include "includes.h"

#include <QApplication>
#include <QHostInfo>
#include <QUrl>

namespace NekoGui_fmt {
    AbstractBean::AbstractBean(int version) {
        this->version = version;
        _add(new configItem("_v", &this->version, itemType::integer));
        _add(new configItem("name", &name, itemType::string));
        _add(new configItem("addr", &serverAddress, itemType::string));
        _add(new configItem("port", &serverPort, itemType::integer));
        _add(new configItem("c_cfg", &custom_config, itemType::string));
        _add(new configItem("c_out", &custom_outbound, itemType::string));
    }

    QString AbstractBean::ToNekorayShareLink(const QString &type) {
        auto b = ToJson();
        QUrl url;
        url.setScheme("nekoray");
        url.setHost(type);
        url.setFragment(QJsonObject2QString(b, true)
                            .toUtf8()
                            .toBase64(QByteArray::Base64UrlEncoding));
        return url.toString();
    }

    QString AbstractBean::DisplayAddress() {
        return ::DisplayAddress(serverAddress, serverPort);
    }

    QString AbstractBean::DisplayName() {
        if (name.isEmpty()) {
            return DisplayAddress();
        }
        return name;
    }

    QString AbstractBean::DisplayTypeAndName() {
        return QString("[%1] %2").arg(DisplayType(), DisplayName());
    }

    void AbstractBean::ResolveDomainToIP(const std::function<void()> &onFinished) {
        bool noResolve = false;
        if (dynamic_cast<ChainBean *>(this) != nullptr) noResolve = true;
        if (dynamic_cast<CustomBean *>(this) != nullptr) noResolve = true;
        if (dynamic_cast<NaiveBean *>(this) != nullptr) noResolve = true;
        if (IsIpAddress(serverAddress)) noResolve = true;
        if (noResolve) {
            onFinished();
            return;
        }

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0) // TODO older QT
        QHostInfo::lookupHost(serverAddress, QApplication::instance(), [=](const QHostInfo &host) {
            auto addr = host.addresses();
            if (!addr.isEmpty()) {
                auto domain = serverAddress;
                auto stream = GetStreamSettings(this);

                // replace serverAddress
                serverAddress = addr.first().toString();

                // replace ws tls
                if (stream != nullptr) {
                    if (stream->security == "tls" && stream->sni.isEmpty()) {
                        stream->sni = domain;
                    }
                    if (stream->network == "ws" && stream->host.isEmpty()) {
                        stream->host = domain;
                    }
                }
            }
            onFinished();
        });
#endif
    }
} // namespace NekoGui_fmt
