#pragma once

#include "fmt/AbstractBean.hpp"
#include "fmt/V2RayStreamSettings.hpp"

namespace NekoRay::fmt {
    class VMessBean : public AbstractBean {
    public:
        QString uuid = "";
        int aid = 0;
        QString security = "auto";

        QSharedPointer<V2rayStreamSettings> stream = QSharedPointer<V2rayStreamSettings>(new V2rayStreamSettings());

        VMessBean() : AbstractBean(0) {
            _add(new configItem("id", &uuid, itemType::string));
            _add(new configItem("aid", &aid, itemType::integer));
            _add(new configItem("sec", &security, itemType::string));
            _add(new configItem("stream", dynamic_cast<JsonStore *>(stream.get()), itemType::jsonStore));
        };

        QString DisplayType() override { return "VMess"; };

        CoreObjOutboundBuildResult BuildCoreObjV2Ray() override;

        CoreObjOutboundBuildResult BuildCoreObjSingBox() override;

        bool TryParseLink(const QString &link);

        QString ToShareLink() override;
    };
} // namespace NekoRay::fmt
