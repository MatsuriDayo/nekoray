#pragma once

#include "fmt/AbstractBean.hpp"

namespace NekoGui_fmt {
    class NaiveBean : public AbstractBean {
    public:
        QString username = "";
        QString password = "";
        QString protocol = "https";
        QString extra_headers = "";
        QString sni = "";
        QString certificate = "";
        int insecure_concurrency = 0;

        bool disable_log = false;

        NaiveBean() : AbstractBean(0) {
            _add(new configItem("username", &username, itemType::string));
            _add(new configItem("password", &password, itemType::string));
            _add(new configItem("protocol", &protocol, itemType::string));
            _add(new configItem("extra_headers", &extra_headers, itemType::string));
            _add(new configItem("sni", &sni, itemType::string));
            _add(new configItem("certificate", &certificate, itemType::string));
            _add(new configItem("insecure_concurrency", &insecure_concurrency, itemType::integer));
            _add(new configItem("disable_log", &disable_log, itemType::boolean));
        };

        QString DisplayCoreType() override { return "Naive"; };

        QString DisplayType() override { return "Naive"; };

        int NeedExternal(bool isFirstProfile) override;

        ExternalBuildResult BuildExternal(int mapping_port, int socks_port, int external_stat) override;

        bool TryParseLink(const QString &link);

        QString ToShareLink() override;
    };
} // namespace NekoGui_fmt