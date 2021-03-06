// DO NOT INCLUDE THIS

namespace NekoRay {

    class Routing : public JsonStore {
    public:
        QString direct_ip;
        QString direct_domain;
        QString proxy_ip;
        QString proxy_domain;
        QString block_ip;
        QString block_domain;
        QString custom = "{\"rules\": []}";

        explicit Routing(int preset = 0);

        QString toString() const;

        static QStringList List();

        static void SetToActive(const QString &name);
    };

    class ExtraCore : public JsonStore {
    public:
        QString core_map;

        explicit ExtraCore();

        [[nodiscard]] QString Get(const QString &id) const;

        void Set(const QString &id, const QString &path);

        void Delete(const QString &id);
    };

    class DataStore : public JsonStore {
    public:
        // Running

        QString core_token;
        int core_port = 19810;
        int started_id = -1919;

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
        int mux_cool = -8;
        QString theme = "0";
        QString v2ray_asset_dir = "";
        int language = 0;
        QString mw_size = "";

        // Security
        bool insecure_hint = true;
        bool skip_cert = false;

        // Remember
        bool system_proxy = false;
        int remember_id = -1919;
        bool remember_enable = false;
        bool start_minimal = false;

        // Socks & HTTP Inbound
        QString inbound_address = "127.0.0.1";
        int inbound_socks_port = 2080;
        int inbound_http_port = -2081;
        QString custom_inbound = "{\"inbounds\": []}";

        // DNS
        QString remote_dns = "https://1.0.0.1/dns-query";
        QString direct_dns = "https+local://223.5.5.5/dns-query";
        bool dns_routing = true;
        bool enhance_resolve_server_domain = false;

        // Routing
        bool fake_dns = false;
        QString domain_strategy = "AsIs";
        QString outbound_domain_strategy = "AsIs";
        int sniffing_mode = SniffingMode::DISABLE;
        int domain_matcher = DomainMatcher::MPH;
        QString custom_route_global = "{\"rules\": []}";
        QString active_routing = "Default";

        // Hotkey
        QString hotkey_mainwindow = "";
        QString hotkey_group = "";
        QString hotkey_route = "";

        // Other Core
        ExtraCore *extraCore = new ExtraCore;

        // Running Cache

        Routing *routing = new Routing;
        int imported_count = 0;
        bool refreshing_group_list = false;

        // Running Flags

        bool flag_use_appdata = false;
        bool flag_many = false;

        //

        DataStore();

        void UpdateStartedId(int id);
    };

    extern DataStore *dataStore;

}
