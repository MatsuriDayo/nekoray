#pragma once

#include "NekoRay.hpp"

namespace NekoRay::fmt {
    class V2rayStreamSettings : public JsonStore {
    public:
        QString network = "tcp";
        QString security = "none";
        // ws/h2/grpc
        QString path = "";
        QString host = "";
        // ws
        int max_early_data = 0;
        QString early_data_header_name = "";
        // QUIC & KCP
        QString header_type = "";
        // tls
        QString sni = "";
        QString certificate = "";
        bool allow_insecure = false;

        V2rayStreamSettings() : JsonStore() {
            _add(new configItem("network", &network, itemType::string));
            _add(new configItem("security", &security, itemType::string));
            _add(new configItem("path", &path, itemType::string));
            _add(new configItem("host", &host, itemType::string));
            _add(new configItem("sni", &sni, itemType::string));
            _add(new configItem("certificate", &certificate, itemType::string));
            _add(new configItem("allow_insecure", &allow_insecure, itemType::boolean));
            _add(new configItem("max_early_data", &max_early_data, itemType::integer));
            _add(new configItem("early_data_header_name", &early_data_header_name, itemType::string));
            _add(new configItem("header_type", &header_type, itemType::string));
        }

        QJsonObject BuildStreamSettings();
    };
}

