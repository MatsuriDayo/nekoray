#pragma once

#include "fmt/AbstractBean.hpp"
#include "fmt/V2RayStreamSettings.hpp"

namespace NekoRay::fmt {
    class ShadowTLSBean : public AbstractBean {
    public:
        QString password = "";
        int shadowTLSVersion = 3;

        QSharedPointer<V2rayStreamSettings> stream = QSharedPointer<V2rayStreamSettings>(new V2rayStreamSettings());

        ShadowTLSBean() : AbstractBean(0) {
            _add(new configItem("pass", &password, itemType::string));
            _add(new configItem("shadowTLSVersion", &shadowTLSVersion, itemType::integer));
            _add(new configItem("stream", dynamic_cast<JsonStore *>(stream.get()), itemType::jsonStore));
        };

        QString DisplayType() override { return "ShadowTLS"; };

        CoreObjOutboundBuildResult BuildCoreObjSingBox() override;
    };
} // namespace NekoRay::fmt
