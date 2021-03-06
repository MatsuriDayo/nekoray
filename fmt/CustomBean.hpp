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

        QString DisplayType() override { return core; };

        bool NeedExternal() override { return true; };

        ExternalBuildResult BuildExternal(int mapping_port, int socks_port) override;
    };
}