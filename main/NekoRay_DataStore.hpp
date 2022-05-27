// DO NOT INCLUDE THIS

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMap>
#include <QList>
#include <QFile>
#include <QDir>

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

    class ExtraCore : public JsonStore {
    public:
        QString core_map;

        explicit ExtraCore();

        QString Get(const QString &id) const;

        void Set(const QString &id, const QString &path);

        void Delete(const QString &id);
    };

    class DataStore : public JsonStore {
    public:
        // Running

        QString core_token;
        int core_port = 19810;
        int started_id = -1919;
        bool system_proxy = false;

        // Saved

        // Misc
        QString core_path = "../nekoray_core";
        QString core_cap_path = "../nekoray_core_cap";
        QString log_level = "warning";
        QString user_agent = "ClashForAndroid/2.5.9.premium";
        bool sub_use_proxy = false;
        QString test_url = "truehttp://cp.cloudflare.com/";
        int test_concurrent = 5;
        int traffic_loop_interval = 500;
        int current_group = 0; //group id
        int mux_cool = 8;
        QString theme = "0";
        QString v2ray_asset_dir = "";

        // Socks & HTTP Inbound
        QString inbound_address = "127.0.0.1";
        int inbound_socks_port = 2080;
        int inbound_http_port = -2081;
        QString custom_inbound = "{\n"
                                 "    \"inbounds\": []\n"
                                 "}";

        // DNS
        QString remote_dns = "https://1.0.0.1/dns-query";
        QString direct_dns = "https+local://223.5.5.5/dns-query";
        bool dns_routing = true;

        // Routing
        QString domain_strategy = "AsIs";
        QString outbound_domain_strategy = "AsIs";
        int sniffing_mode = SniffingMode::DISABLE;
        int domain_matcher = DomainMatcher::MPH;
        QString custom_route = "{\n"
                               "    \"rules\": []\n"
                               "}";
        Routing *routing = new Routing;

        // Other Core
        ExtraCore *extraCore = new ExtraCore;

        DataStore();

        // Running Cache

        int updated_count = 0;
        bool refreshing_group_list = false;
    };

    extern DataStore *dataStore;

}
