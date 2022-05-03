#ifndef NEKORAY_V2RAYSTREAMSETTINGS_HPP
#define NEKORAY_V2RAYSTREAMSETTINGS_HPP

#include "NekoRay.hpp"

namespace NekoRay::fmt {
    class V2rayStreamSettings : public JsonStore {
    public:
        QString network = "tcp";
        QString security = "none";
        // ws/h2/grpc
        QString path = "";
        QString host = "";
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
        }

        QJsonObject BuildStreamSettings();
    };
}

#endif //NEKORAY_V2RAYSTREAMSETTINGS_HPP
