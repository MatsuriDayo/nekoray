// DO NOT INCLUDE THIS

#include "NekoRay_ConfigItem.hpp"
#include "Const.hpp"

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMap>
#include <QList>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

namespace NekoRay {

    class Routing : public JsonStore {
    public:
        QString direct_ip;
        QString direct_domain;
        QString proxy_ip;
        QString proxy_domain;
        QString block_ip;
        QString block_domain;

        explicit Routing(int preset = 0);
    };

    class DataStore : public JsonStore {
    public:
        // Running

        QString core_token;
        int started_id = -1919;
        bool system_proxy = false;

        // Saved

        // Misc
        QString core_path = "./nekoray_core";
        QString log_level = "warning";
        QString user_agent = "ClashForAndroid/2.5.8";
        int traffic_loop_interval = 500;
        int current_group = 0; //group id
        int mux_cool = 8;

        // Socks & HTTP Inbound
        QString inbound_address = "127.0.0.1";
        int inbound_socks_port = 2080;
        int inbound_http_port = -2081;

        // DNS
        QString remote_dns = "https://1.0.0.1/dns-query";
        QString direct_dns = "https://223.5.5.5/dns-query";
        bool dns_routing = true;

        // Routing
        QString domain_strategy = "AsIs";
        QString outbound_domain_strategy = "AsIs";
        int sniffing_mode = SniffingMode::DISABLE;
        int ipv6_mode = IPv6Mode::DISABLE;
        int domain_matcher = DomainMatcher::MPH;
        Routing *routing = new Routing;

        DataStore() : JsonStore("groups/nekoray.json") {
            _add(new configItem("core_path", &core_path, itemType::string));
            _add(new configItem("user_agent", &user_agent, itemType::string));
            _add(new configItem("current_group", &current_group, itemType::integer));
            _add(new configItem("inbound_address", &inbound_address, itemType::string));
            _add(new configItem("inbound_socks_port", &inbound_socks_port, itemType::integer));
            _add(new configItem("inbound_http_port", &inbound_http_port, itemType::integer));
            _add(new configItem("log_level", &log_level, itemType::string));
            _add(new configItem("remote_dns", &remote_dns, itemType::string));
            _add(new configItem("direct_dns", &direct_dns, itemType::string));
            _add(new configItem("domain_strategy", &domain_strategy, itemType::string));
            _add(new configItem("outbound_domain_strategy", &outbound_domain_strategy, itemType::string));
            _add(new configItem("sniffing_mode", &sniffing_mode, itemType::integer));
            _add(new configItem("mux_cool", &mux_cool, itemType::integer));
            _add(new configItem("ipv6_mode", &ipv6_mode, itemType::integer));
            _add(new configItem("routing", dynamic_cast<JsonStore *>(routing), itemType::jsonStore));
            _add(new configItem("traffic_loop_interval", &traffic_loop_interval, itemType::integer));
            _add(new configItem("dns_routing", &dns_routing, itemType::boolean));
        }

        // Cache
        int updated_count = 0;
    };

    extern DataStore *dataStore;

}
