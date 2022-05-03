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

        explicit AbstractBean(int version);

        QString ToNekorayShareLink(const QString &type);

        [[nodiscard]] virtual QString DisplayAddress();

        [[nodiscard]] virtual QString DisplayName();

        virtual QString DisplayType() { return {}; };

        virtual QString DisplayTypeAndName();

        virtual bool NeedExternal() { return false; };

        virtual CoreObjOutboundBuildResult BuildCoreObj() { return {}; };

        virtual ExternalBuildResult BuildExternal(int mapping_port, int socks_port) { return {}; };

        virtual QString ToShareLink() { return {}; };

        virtual QString InsecureHint() { return {}; };

    };

    QString DisplayInsecureHint(const QSharedPointer<AbstractBean> &);

}
