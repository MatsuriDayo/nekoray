#pragma once

#include "AbstractBean.hpp"

namespace NekoRay::fmt {
    class V2rayStreamSettings : public JsonStore {
    public:
        QString network = "tcp";
        QString security = "none";
        QString packet_encoding = "";
        // ws/h2/grpc/tcp-http
        QString path = "";
        QString host = "";
        // kcp/quic/tcp-http
        QString header_type = "";
        // tls
        QString sni = "";
        QString alpn = "";
        QString certificate = "";
        bool allow_insecure = false;

        V2rayStreamSettings() : JsonStore() {
            _add(new configItem("net", &network, itemType::string));
            _add(new configItem("sec", &security, itemType::string));
            _add(new configItem("pac_enc", &packet_encoding, itemType::string));
            _add(new configItem("path", &path, itemType::string));
            _add(new configItem("host", &host, itemType::string));
            _add(new configItem("sni", &sni, itemType::string));
            _add(new configItem("alpn", &alpn, itemType::string));
            _add(new configItem("cert", &certificate, itemType::string));
            _add(new configItem("insecure", &allow_insecure, itemType::boolean));
            _add(new configItem("h_type", &header_type, itemType::string));
        }

        QJsonObject BuildStreamSettings();

        [[nodiscard]] QString InsecureHint() const;
    };

    inline V2rayStreamSettings *GetStreamSettings(const QSharedPointer<AbstractBean> &bean) {
        if (bean == nullptr) return nullptr;
        auto stream_item = bean->_get("stream");
        if (stream_item != nullptr) {
            auto stream_store = (NekoRay::JsonStore *) stream_item->ptr;
            auto stream = (NekoRay::fmt::V2rayStreamSettings *) stream_store;
            return stream;
        }
        return nullptr;
    }
}
