#pragma once

#include "fmt/AbstractBean.hpp"

namespace NekoRay::fmt {
    class NaiveBean : public AbstractBean {
    public:
        QString username = "";
        QString password = "";
        QString protocol = "https";
        QString extra_headers = "";
        QString sni = "";
        QString certificate = "";
        int insecure_concurrency = 0;

        NaiveBean() : AbstractBean(0) {
            _add(new configItem("username", &username, itemType::string));
            _add(new configItem("password", &password, itemType::string));
            _add(new configItem("protocol", &protocol, itemType::string));
            _add(new configItem("extra_headers", &extra_headers, itemType::string));
            _add(new configItem("sni", &sni, itemType::string));
            _add(new configItem("certificate", &certificate, itemType::string));
            _add(new configItem("insecure_concurrency", &insecure_concurrency, itemType::integer));
        };

        QString DisplayType() override { return "Naive"; };

        bool NeedExternal() override { return true; };

        ExternalBuildResult BuildExternal(int mapping_port, int socks_port) override;

        bool TryParseLink(const QString &link);

        QString ToShareLink() override;
    };
}