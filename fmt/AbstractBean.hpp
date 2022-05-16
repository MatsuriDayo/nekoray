#pragma once

#include "main/NekoRay.hpp"

namespace NekoRay::fmt {

    struct CoreObjOutboundBuildResult {
    public:
        QJsonObject outbound;
        QString error;
    };

    struct ExternalBuildResult {
    public:
        QString program;
        QStringList env;
        QStringList arguments;
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
            _add(new configItem("_v", &this->version, itemType::integer));
            _add(new configItem("name", &name, itemType::string));
            _add(new configItem("addr", &serverAddress, itemType::string));
            _add(new configItem("port", &serverPort, itemType::integer));
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

        virtual ExternalBuildResult BuildExternal(int mapping_port, int socks_port) { return {}; };

        virtual QString ToShareLink() { return ""; };

    };

}
