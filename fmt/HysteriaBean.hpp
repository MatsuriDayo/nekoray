#pragma once

#include "fmt/AbstractBean.hpp"

namespace NekoRay::fmt {
    class HysteriaBean : public AbstractBean {
    public:
        static constexpr int hysteria_protocol_udp = 0;
        static constexpr int hysteria_protocol_facktcp = 1;
        static constexpr int hysteria_protocol_wechat_video = 2;

        int protocol = 0;

        //

        static constexpr int hysteria_auth_none = 0;
        static constexpr int hysteria_auth_string = 1;
        static constexpr int hysteria_auth_base64 = 2;

        int authPayloadType = 0;
        QString authPayload = "";
        QString obfsPassword = "";

        //

        int uploadMbps = 100;
        int downloadMbps = 100;

        qint64 streamReceiveWindow = 0;
        qint64 connectionReceiveWindow = 0;
        bool disableMtuDiscovery = false;

        bool allowInsecure = false;
        QString sni = "";
        QString alpn = ""; // only 1
        QString caText = "";

        //

        int hopInterval = 10;
        QString hopPort = "";

        HysteriaBean() : AbstractBean(0) {
            _add(new configItem("protocol", &protocol, itemType::integer));
            _add(new configItem("authPayloadType", &authPayloadType, itemType::integer));
            _add(new configItem("authPayload", &authPayload, itemType::string));
            _add(new configItem("obfsPassword", &obfsPassword, itemType::string));
            _add(new configItem("uploadMbps", &uploadMbps, itemType::integer));
            _add(new configItem("downloadMbps", &downloadMbps, itemType::integer));
            _add(new configItem("streamReceiveWindow", &streamReceiveWindow, itemType::integer64));
            _add(new configItem("connectionReceiveWindow", &connectionReceiveWindow, itemType::integer64));
            _add(new configItem("disableMtuDiscovery", &disableMtuDiscovery, itemType::boolean));
            _add(new configItem("allowInsecure", &allowInsecure, itemType::boolean));
            _add(new configItem("sni", &sni, itemType::string));
            _add(new configItem("alpn", &alpn, itemType::string));
            _add(new configItem("caText", &caText, itemType::string));
            _add(new configItem("hopInterval", &hopInterval, itemType::integer));
            _add(new configItem("hopPort", &hopPort, itemType::string));
        };

        QString DisplayAddress() override {
            if (!hopPort.trimmed().isEmpty()) return WrapIPV6Host(serverAddress) + ":" + hopPort;
            return ::DisplayAddress(serverAddress, serverPort);
        }

        QString DisplayCoreType() override { return NeedExternal(false) == 0 ? software_core_name : "Hysteria"; };

        QString DisplayType() override { return "Hysteria"; };

        int NeedExternal(bool isFirstProfile) override;

        ExternalBuildResult BuildExternal(int mapping_port, int socks_port, int external_stat) override;

        CoreObjOutboundBuildResult BuildCoreObjSingBox() override;

        bool TryParseLink(const QString &link);

        QString ToShareLink() override;
    };
} // namespace NekoRay::fmt