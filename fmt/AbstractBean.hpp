#pragma once

#include "NekoRay.hpp"

namespace NekoRay::fmt {

    class CoreObjOutboundBuildResult {
    public:
        QJsonObject outbound;
        QString error;
    };

    class AbstractBean : public JsonStore {
    public:
        int version;
        QString name = "";
        QString serverAddress = "127.0.0.1";
        int serverPort = 1080;

        // hidden
        bool isFirstProfile = false;

        explicit AbstractBean(int version) {
            this->version = version;
            _add(new configItem("version", &this->version, itemType::integer));
            _add(new configItem("name", &name, itemType::string));
            _add(new configItem("serverAddress", &serverAddress, itemType::string));
            _add(new configItem("serverPort", &serverPort, itemType::integer));
        };

        [[nodiscard]] virtual QString DisplayAddress() {
            return ::DisplayAddress(serverAddress, serverPort);
        };

        [[nodiscard]] virtual QString DisplayName() {
            if (name.isEmpty()) {
                return DisplayAddress();
            }
            return name;
        };

        virtual QString DisplayType() { return ""; };

        virtual QString DisplayTypeAndName() { return QString(" [%1] %2").arg(DisplayType(), DisplayName()); };

        virtual bool NeedExternal() { return false; };

        virtual CoreObjOutboundBuildResult BuildCoreObj() { return {}; };

        virtual QString ToShareLink() { return ""; };

    };

}
