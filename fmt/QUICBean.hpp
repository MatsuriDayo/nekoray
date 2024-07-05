#pragma once

#include "fmt/AbstractBean.hpp"

namespace NekoGui_fmt {
    class QUICBean : public AbstractBean {
    public:
        // static constexpr int proxy_Hysteria = 0;
        static constexpr int proxy_TUIC = 1;
        static constexpr int proxy_Hysteria2 = 3;
        int proxy_type = proxy_Hysteria2;

        bool forceExternal = false;

        // Hysteria 2

        QString obfsPassword = "";

        int uploadMbps = 0;
        int downloadMbps = 0;

        qint64 streamReceiveWindow = 0;
        qint64 connectionReceiveWindow = 0;
        bool disableMtuDiscovery = false;

        int hopInterval = 10;
        QString hopPort = "";

        // TUIC

        QString uuid = "";
        QString congestionControl = "bbr";
        QString udpRelayMode = "native";
        bool zeroRttHandshake = false;
        QString heartbeat = "10s";
        bool uos = false;

        // HY2&TUIC

        QString password = "";

        // TLS

        bool allowInsecure = false;
        QString sni = "";
        QString alpn = "";
        QString caText = "";
        bool disableSni = false;

        explicit QUICBean(int _proxy_type) : AbstractBean(0) {
            proxy_type = _proxy_type;
            if (proxy_type == proxy_Hysteria2) {
                _add(new configItem("obfsPassword", &obfsPassword, itemType::string));
                _add(new configItem("uploadMbps", &uploadMbps, itemType::integer));
                _add(new configItem("downloadMbps", &downloadMbps, itemType::integer));
                _add(new configItem("streamReceiveWindow", &streamReceiveWindow, itemType::integer64));
                _add(new configItem("connectionReceiveWindow", &connectionReceiveWindow, itemType::integer64));
                _add(new configItem("disableMtuDiscovery", &disableMtuDiscovery, itemType::boolean));
                _add(new configItem("hopInterval", &hopInterval, itemType::integer));
                _add(new configItem("hopPort", &hopPort, itemType::string));
                _add(new configItem("password", &password, itemType::string));
            } else if (proxy_type == proxy_TUIC) {
                _add(new configItem("uuid", &uuid, itemType::string));
                _add(new configItem("password", &password, itemType::string));
                _add(new configItem("congestionControl", &congestionControl, itemType::string));
                _add(new configItem("udpRelayMode", &udpRelayMode, itemType::string));
                _add(new configItem("zeroRttHandshake", &zeroRttHandshake, itemType::boolean));
                _add(new configItem("heartbeat", &heartbeat, itemType::string));
                _add(new configItem("uos", &uos, itemType::boolean));
            }
            _add(new configItem("forceExternal", &forceExternal, itemType::boolean));
            // TLS
            _add(new configItem("allowInsecure", &allowInsecure, itemType::boolean));
            _add(new configItem("sni", &sni, itemType::string));
            _add(new configItem("alpn", &alpn, itemType::string));
            _add(new configItem("caText", &caText, itemType::string));
            _add(new configItem("disableSni", &disableSni, itemType::boolean));
        };

        QString DisplayAddress() override {
            if (!hopPort.trimmed().isEmpty()) return WrapIPV6Host(serverAddress) + ":" + hopPort;
            return ::DisplayAddress(serverAddress, serverPort);
        }

        QString DisplayCoreType() override {
            if (NeedExternal(true) == 0) {
                return software_core_name;
            } else if (proxy_type == proxy_TUIC) {
                return "tuic";
            } else {
                return "hysteria2";
            }
        }

        QString DisplayType() override {
            if (proxy_type == proxy_TUIC) {
                return "TUIC";
            } else {
                return "Hysteria2";
            }
        };

        int NeedExternal(bool isFirstProfile) override;

        ExternalBuildResult BuildExternal(int mapping_port, int socks_port, int external_stat) override;

        CoreObjOutboundBuildResult BuildCoreObjSingBox() override;

        bool TryParseLink(const QString &link);

        QString ToShareLink() override;
    };
} // namespace NekoGui_fmt