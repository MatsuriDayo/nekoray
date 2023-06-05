#pragma once

#include "fmt/AbstractBean.hpp"

namespace NekoGui_fmt {
    class CustomBean : public AbstractBean {
    public:
        QString core;
        QList<QString> command;
        QString config_suffix;
        QString config_simple;
        int mapping_port = 0;
        int socks_port = 0;

        CustomBean() : AbstractBean(0) {
            _add(new configItem("core", &core, itemType::string));
            _add(new configItem("cmd", &command, itemType::stringList));
            _add(new configItem("cs", &config_simple, itemType::string));
            _add(new configItem("cs_suffix", &config_suffix, itemType::string));
            _add(new configItem("mapping_port", &mapping_port, itemType::integer));
            _add(new configItem("socks_port", &socks_port, itemType::integer));
        };

        QString DisplayType() override {
            if (core == "internal") {
                auto obj = QString2QJsonObject(config_simple);
                return obj[IS_NEKO_BOX ? "type" : "protocol"].toString();
            } else if (core == "internal-full") {
                return software_core_name + " config";
            }
            return core;
        };

        QString DisplayCoreType() override { return NeedExternal(true) == 0 ? software_core_name : core; };

        QString DisplayAddress() override {
            if (core == "internal") {
                auto obj = QString2QJsonObject(config_simple);
                if (IS_NEKO_BOX) {
                    return ::DisplayAddress(obj["server"].toString(), obj["server_port"].toInt());
                } else {
                    return {};
                }
            } else if (core == "internal-full") {
                return {};
            }
            return AbstractBean::DisplayAddress();
        };

        int NeedExternal(bool isFirstProfile) override;

        ExternalBuildResult BuildExternal(int mapping_port, int socks_port, int external_stat) override;

        CoreObjOutboundBuildResult BuildCoreObjSingBox() override;

        CoreObjOutboundBuildResult BuildCoreObjV2Ray() override;
    };
} // namespace NekoGui_fmt