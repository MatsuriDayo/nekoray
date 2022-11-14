#pragma once

#include "fmt/AbstractBean.hpp"

namespace NekoRay::fmt {
    class CustomBean : public AbstractBean {
    public:
        QString core;
        QList<QString> command;
        QString config_suffix;
        QString config_simple;

        CustomBean() : AbstractBean(0) {
            _add(new configItem("core", &core, itemType::string));
            _add(new configItem("cmd", &command, itemType::stringList));
            _add(new configItem("cs", &config_simple, itemType::string));
            _add(new configItem("cs_suffix", &config_suffix, itemType::string));
        };

        QString DisplayType() override {
            if (core == "internal") {
                auto obj = QString2QJsonObject(config_simple);
                return obj[IS_NEKO_BOX ? "type" : "protocol"].toString();
            }
            return core;
        };

        QString DisplayCoreType() override { return NeedExternal(false, false) ? core : software_core_name; };

        QString DisplayAddress() override {
            if (core == "internal") {
                auto obj = QString2QJsonObject(config_simple);
                if (IS_NEKO_BOX) {
                    return ::DisplayAddress(obj["server"].toString(), obj["server_port"].toInt());
                } else {
                    return {};
                }
            }
            return AbstractBean::DisplayAddress();
        };

        int NeedExternal(bool isFirstProfile, bool isVPN) override;

        ExternalBuildResult BuildExternal(int mapping_port, int socks_port, int external_stat) override;

        CoreObjOutboundBuildResult BuildCoreObjSingBox() override;

        CoreObjOutboundBuildResult BuildCoreObjV2Ray() override;
    };
}