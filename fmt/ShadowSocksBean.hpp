#pragma once

#include "fmt/AbstractBean.hpp"
#include "fmt/V2RayStreamSettings.hpp"

namespace NekoGui_fmt {
    class ShadowSocksBean : public AbstractBean {
    public:
        QString method = "aes-128-gcm";
        QString password = "";
        QString plugin = "";
        int uot = 0;

        std::shared_ptr<V2rayStreamSettings> stream = std::make_shared<V2rayStreamSettings>();

        ShadowSocksBean() : AbstractBean(0) {
            _add(new configItem("method", &method, itemType::string));
            _add(new configItem("pass", &password, itemType::string));
            _add(new configItem("plugin", &plugin, itemType::string));
            _add(new configItem("uot", &uot, itemType::integer));
            _add(new configItem("stream", dynamic_cast<JsonStore *>(stream.get()), itemType::jsonStore));
        };

        QString DisplayType() override { return "Shadowsocks"; };

        CoreObjOutboundBuildResult BuildCoreObjV2Ray() override;

        CoreObjOutboundBuildResult BuildCoreObjSingBox() override;

        bool TryParseLink(const QString &link);

        QString ToShareLink() override;
    };
} // namespace NekoGui_fmt
