#pragma once

#include "fmt/AbstractBean.hpp"
#include "fmt/comm/V2RayStreamSettings.hpp"

namespace NekoRay::fmt {
    class SocksBean : public AbstractBean {
    public:
        int socksVersion = 5;
        QString username = "";
        QString password = "";

        QSharedPointer<V2rayStreamSettings> stream = QSharedPointer<V2rayStreamSettings>(new V2rayStreamSettings());
        QString custom = "";

        SocksBean() : AbstractBean(0) {
            _add(new configItem("socksVersion", &socksVersion, itemType::integer));
            _add(new configItem("username", &username, itemType::string));
            _add(new configItem("password", &password, itemType::string));
            _add(new configItem("stream", dynamic_cast<JsonStore *>(stream.get()), itemType::jsonStore));
            _add(new configItem("custom", &custom, itemType::string));
        };

        QString DisplayType() override { return "Socks"; };

        CoreObjOutboundBuildResult BuildCoreObj() override;

        bool TryParseLink(const QString &link);

        QString ToShareLink() override;
    };
}
