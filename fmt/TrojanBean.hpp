#pragma once

#include "fmt/AbstractBean.hpp"
#include "fmt/V2RayStreamSettings.hpp"

namespace NekoRay::fmt {
    class TrojanBean : public AbstractBean {
    public:
        QString password = "";

        QSharedPointer<V2rayStreamSettings> stream = QSharedPointer<V2rayStreamSettings>(new V2rayStreamSettings());
        QString custom = "";

        TrojanBean() : AbstractBean(0) {
            _add(new configItem("pass", &password, itemType::string));
            _add(new configItem("stream", dynamic_cast<JsonStore *>(stream.get()), itemType::jsonStore));
            _add(new configItem("custom", &custom, itemType::string));
        };

        QString DisplayType() override { return "Trojan"; };

        CoreObjOutboundBuildResult BuildCoreObj() override;

        bool TryParseLink(const QString &link);

        QString ToShareLink() override;
    };
}