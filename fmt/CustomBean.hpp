#pragma once

#include "fmt/AbstractBean.hpp"

namespace NekoRay::fmt {
    class CustomBean : public AbstractBean {
    public:
        QString core;
        QList<QString> command;
//        QString config_map; // map: fn to text
        QString config_simple;

        CustomBean() : AbstractBean(0) {
            _add(new configItem("core", &core, itemType::string));
            _add(new configItem("cmd", &command, itemType::stringList));
//            _add(new configItem("cm", &config_map, itemType::string));
            _add(new configItem("cs", &config_simple, itemType::string));
        };

        QString DisplayType() override {
            if (core == "internal") {
                auto obj = QString2QJsonObject(config_simple);
                return obj[IS_NEKO_BOX ? "type" : "protocol"].toString();
            }
            return core;
        };

        QString DisplayCoreType() override { return NeedExternal() ? core : software_core_name; };

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

        bool NeedExternal() override {
            if (core == "internal") return false;
            if (IS_NEKO_BOX && core == "hysteria") return false;
            return true;
        };

        ExternalBuildResult BuildExternal(int mapping_port, int socks_port) override;

        CoreObjOutboundBuildResult BuildCoreObjSingBox() override;

        CoreObjOutboundBuildResult BuildCoreObjV2Ray() override;
    };
}