#pragma once

#include "fmt/AbstractBean.hpp"
#include "fmt/comm/V2RayStreamSettings.hpp"

namespace NekoRay::fmt {
    class ShadowSocksBean : public AbstractBean {
    public:
        QString method = "aes-128-gcm";
        QString password = "";
        QString plugin = "";

        QSharedPointer<V2rayStreamSettings> stream = QSharedPointer<V2rayStreamSettings>(new V2rayStreamSettings());
        QString custom = "";

        ShadowSocksBean() : AbstractBean(0) {
            _add(new configItem("method", &method, itemType::string));
            _add(new configItem("password", &password, itemType::string));
            _add(new configItem("plugin", &plugin, itemType::string));
            _add(new configItem("stream", dynamic_cast<JsonStore *>(stream.get()), itemType::jsonStore));
            _add(new configItem("custom", &custom, itemType::string));
        };

        QString DisplayType() override { return "ShadowSocks"; };

        CoreObjOutboundBuildResult BuildCoreObj() override;

        bool TryParseLink(const QString &link);

        QString ToShareLink() override;
    };
}
