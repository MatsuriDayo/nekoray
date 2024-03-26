#include "NekoGui.hpp"
#include "fmt/Preset.hpp"

#include <QFile>
#include <QDir>
#include <QApplication>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#ifdef Q_OS_WIN
#include "sys/windows/guihelper.h"
#else
#ifdef Q_OS_LINUX
#include <sys/linux/LinuxCap.h>
#endif
#include <unistd.h>
#endif

namespace NekoGui_ConfigItem {

    // 添加关联
    void JsonStore::_add(configItem *item) {
        _map.insert(item->name, std::shared_ptr<configItem>(item));
    }

    QString JsonStore::_name(void *p) {
        for (const auto &_item: _map) {
            if (_item->ptr == p) return _item->name;
        }
        return {};
    }

    std::shared_ptr<configItem> JsonStore::_get(const QString &name) {
        // 直接 [] 会设置一个 nullptr ，所以先判断是否存在
        if (_map.contains(name)) {
            return _map[name];
        }
        return nullptr;
    }

    void JsonStore::_setValue(const QString &name, void *p) {
        auto item = _get(name);
        if (item == nullptr) return;

        switch (item->type) {
            case itemType::string:
                *(QString *) item->ptr = *(QString *) p;
                break;
            case itemType::boolean:
                *(bool *) item->ptr = *(bool *) p;
                break;
            case itemType::integer:
                *(int *) item->ptr = *(int *) p;
                break;
            case itemType::integer64:
                *(long long *) item->ptr = *(long long *) p;
                break;
            // others...
            case stringList:
            case integerList:
            case jsonStore:
                break;
        }
    }

    QJsonObject JsonStore::ToJson(const QStringList &without) {
        QJsonObject object;
        for (const auto &_item: _map) {
            auto item = _item.get();
            if (without.contains(item->name)) continue;
            switch (item->type) {
                case itemType::string:
                    // Allow Empty
                    if (!((QString *) item->ptr)->isEmpty()) {
                        object.insert(item->name, *(QString *) item->ptr);
                    }
                    break;
                case itemType::integer:
                    object.insert(item->name, *(int *) item->ptr);
                    break;
                case itemType::integer64:
                    object.insert(item->name, *(long long *) item->ptr);
                    break;
                case itemType::boolean:
                    object.insert(item->name, *(bool *) item->ptr);
                    break;
                case itemType::stringList:
                    object.insert(item->name, QList2QJsonArray<QString>(*(QList<QString> *) item->ptr));
                    break;
                case itemType::integerList:
                    object.insert(item->name, QList2QJsonArray<int>(*(QList<int> *) item->ptr));
                    break;
                case itemType::jsonStore:
                    // _add 时应关联对应 JsonStore 的指针
                    object.insert(item->name, ((JsonStore *) item->ptr)->ToJson());
                    break;
            }
        }
        return object;
    }

    QByteArray JsonStore::ToJsonBytes() {
        QJsonDocument document;
        document.setObject(ToJson());
        return document.toJson(save_control_compact ? QJsonDocument::Compact : QJsonDocument::Indented);
    }

    void JsonStore::FromJson(QJsonObject object) {
        for (const auto &key: object.keys()) {
            if (_map.count(key) == 0) {
                continue;
            }

            auto value = object[key];
            auto item = _map[key].get();

            if (item == nullptr)
                continue; // 故意忽略

            // 根据类型修改ptr的内容
            switch (item->type) {
                case itemType::string:
                    if (value.type() != QJsonValue::String) {
                        continue;
                    }
                    *(QString *) item->ptr = value.toString();
                    break;
                case itemType::integer:
                    if (value.type() != QJsonValue::Double) {
                        continue;
                    }
                    *(int *) item->ptr = value.toInt();
                    break;
                case itemType::integer64:
                    if (value.type() != QJsonValue::Double) {
                        continue;
                    }
                    *(long long *) item->ptr = value.toDouble();
                    break;
                case itemType::boolean:
                    if (value.type() != QJsonValue::Bool) {
                        continue;
                    }
                    *(bool *) item->ptr = value.toBool();
                    break;
                case itemType::stringList:
                    if (value.type() != QJsonValue::Array) {
                        continue;
                    }
                    *(QList<QString> *) item->ptr = QJsonArray2QListString(value.toArray());
                    break;
                case itemType::integerList:
                    if (value.type() != QJsonValue::Array) {
                        continue;
                    }
                    *(QList<int> *) item->ptr = QJsonArray2QListInt(value.toArray());
                    break;
                case itemType::jsonStore:
                    if (value.type() != QJsonValue::Object) {
                        continue;
                    }
                    ((JsonStore *) item->ptr)->FromJson(value.toObject());
                    break;
            }
        }

        if (callback_after_load != nullptr) callback_after_load();
    }

    void JsonStore::FromJsonBytes(const QByteArray &data) {
        QJsonParseError error{};
        auto document = QJsonDocument::fromJson(data, &error);

        if (error.error != error.NoError) {
            qDebug() << "QJsonParseError" << error.errorString();
            return;
        }

        FromJson(document.object());
    }

    bool JsonStore::Save() {
        if (callback_before_save != nullptr) callback_before_save();
        if (save_control_no_save) return false;

        auto save_content = ToJsonBytes();
        auto changed = last_save_content != save_content;
        last_save_content = save_content;

        QFile file;
        file.setFileName(fn);
        file.open(QIODevice::ReadWrite | QIODevice::Truncate);
        file.write(save_content);
        file.close();

        return changed;
    }

    bool JsonStore::Load() {
        QFile file;
        file.setFileName(fn);

        if (!file.exists() && !load_control_must) {
            return false;
        }

        bool ok = file.open(QIODevice::ReadOnly);
        if (!ok) {
            MessageBoxWarning("error", "can not open config " + fn + "\n" + file.errorString());
        } else {
            last_save_content = file.readAll();
            FromJsonBytes(last_save_content);
        }

        file.close();
        return ok;
    }

} // namespace NekoGui_ConfigItem

namespace NekoGui {

    DataStore *dataStore = new DataStore();

    // datastore

    DataStore::DataStore() : JsonStore() {
        _add(new configItem("extraCore", dynamic_cast<JsonStore *>(extraCore), itemType::jsonStore));
        _add(new configItem("inbound_auth", dynamic_cast<JsonStore *>(inbound_auth), itemType::jsonStore));

        _add(new configItem("user_agent2", &user_agent, itemType::string));
        _add(new configItem("test_url", &test_latency_url, itemType::string));
        _add(new configItem("test_url_dl", &test_download_url, itemType::string));
        _add(new configItem("test_dl_timeout", &test_download_timeout, itemType::integer));
        _add(new configItem("current_group", &current_group, itemType::integer));
        _add(new configItem("inbound_address", &inbound_address, itemType::string));
        _add(new configItem("inbound_socks_port", &inbound_socks_port, itemType::integer));
        _add(new configItem("inbound_http_port", &inbound_http_port, itemType::integer));
        _add(new configItem("log_level", &log_level, itemType::string));
        _add(new configItem("mux_protocol", &mux_protocol, itemType::string));
        _add(new configItem("mux_concurrency", &mux_concurrency, itemType::integer));
        _add(new configItem("mux_padding", &mux_padding, itemType::boolean));
        _add(new configItem("mux_default_on", &mux_default_on, itemType::boolean));
        _add(new configItem("traffic_loop_interval", &traffic_loop_interval, itemType::integer));
        _add(new configItem("test_concurrent", &test_concurrent, itemType::integer));
        _add(new configItem("theme", &theme, itemType::string));
        _add(new configItem("custom_inbound", &custom_inbound, itemType::string));
        _add(new configItem("custom_route", &custom_route_global, itemType::string));
        _add(new configItem("v2ray_asset_dir", &v2ray_asset_dir, itemType::string));
        _add(new configItem("sub_use_proxy", &sub_use_proxy, itemType::boolean));
        _add(new configItem("remember_id", &remember_id, itemType::integer));
        _add(new configItem("remember_enable", &remember_enable, itemType::boolean));
        _add(new configItem("language", &language, itemType::integer));
        _add(new configItem("spmode2", &remember_spmode, itemType::stringList));
        _add(new configItem("skip_cert", &skip_cert, itemType::boolean));
        _add(new configItem("hk_mw", &hotkey_mainwindow, itemType::string));
        _add(new configItem("hk_group", &hotkey_group, itemType::string));
        _add(new configItem("hk_route", &hotkey_route, itemType::string));
        _add(new configItem("hk_spmenu", &hotkey_system_proxy_menu, itemType::string));
        _add(new configItem("fakedns", &fake_dns, itemType::boolean));
        _add(new configItem("active_routing", &active_routing, itemType::string));
        _add(new configItem("mw_size", &mw_size, itemType::string));
        _add(new configItem("conn_stat", &connection_statistics, itemType::boolean));
        _add(new configItem("vpn_impl", &vpn_implementation, itemType::integer));
        _add(new configItem("vpn_mtu", &vpn_mtu, itemType::integer));
        _add(new configItem("vpn_ipv6", &vpn_ipv6, itemType::boolean));
        _add(new configItem("vpn_hide_console", &vpn_hide_console, itemType::boolean));
        _add(new configItem("vpn_strict_route", &vpn_strict_route, itemType::boolean));
        _add(new configItem("vpn_bypass_process", &vpn_rule_process, itemType::string));
        _add(new configItem("vpn_bypass_cidr", &vpn_rule_cidr, itemType::string));
        _add(new configItem("vpn_rule_white", &vpn_rule_white, itemType::boolean));
        _add(new configItem("check_include_pre", &check_include_pre, itemType::boolean));
        _add(new configItem("sp_format", &system_proxy_format, itemType::string));
        _add(new configItem("sub_clear", &sub_clear, itemType::boolean));
        _add(new configItem("sub_insecure", &sub_insecure, itemType::boolean));
        _add(new configItem("sub_auto_update", &sub_auto_update, itemType::integer));
        _add(new configItem("enable_js_hook", &enable_js_hook, itemType::integer));
        _add(new configItem("log_ignore", &log_ignore, itemType::stringList));
        _add(new configItem("start_minimal", &start_minimal, itemType::boolean));
        _add(new configItem("max_log_line", &max_log_line, itemType::integer));
        _add(new configItem("splitter_state", &splitter_state, itemType::string));
        _add(new configItem("utlsFingerprint", &utlsFingerprint, itemType::string));
        _add(new configItem("core_box_clash_api", &core_box_clash_api, itemType::integer));
        _add(new configItem("core_box_clash_api_secret", &core_box_clash_api_secret, itemType::string));
        _add(new configItem("core_box_underlying_dns", &core_box_underlying_dns, itemType::string));
        _add(new configItem("core_ray_direct_dns", &core_ray_direct_dns, itemType::boolean));
        _add(new configItem("core_ray_freedom_domainStrategy", &core_ray_freedom_domainStrategy, itemType::string));
        _add(new configItem("vpn_internal_tun", &vpn_internal_tun, itemType::boolean));
#ifdef Q_OS_WIN
        _add(new configItem("core_ray_windows_disable_auto_interface", &core_ray_windows_disable_auto_interface, itemType::boolean));
#endif
    }

    void DataStore::UpdateStartedId(int id) {
        started_id = id;
        if (remember_enable) {
            remember_id = id;
            Save();
        } else if (remember_id >= 0) {
            remember_id = -1919;
            Save();
        }
    }

    QString DataStore::GetUserAgent(bool isDefault) const {
        if (user_agent.isEmpty()) {
            isDefault = true;
        }
        if (isDefault) {
            QString version = SubStrBefore(NKR_VERSION, "-");
            if (!version.contains(".")) version = "2.0";
            if (IS_NEKO_BOX) {
                return "NekoBox/PC/" + version + " (Prefer ClashMeta Format)";
            } else {
                return "NekoRay/PC/" + version + " (Prefer ClashMeta Format)";
            }
        }
        return user_agent;
    }

    // preset routing
    Routing::Routing(int preset) : JsonStore() {
        if (preset == 1) {
            direct_ip =
                "geoip:cn\n"
                "geoip:private";
            direct_domain = "geosite:cn";
            proxy_ip = "";
            proxy_domain = "";
            block_ip = "";
            block_domain =
                "geosite:category-ads-all\n"
                "domain:appcenter.ms\n"
                "domain:firebase.io\n"
                "domain:crashlytics.com\n";
        }
        if (IS_NEKO_BOX) {
            if (!Preset::SingBox::DomainStrategy.contains(domain_strategy)) domain_strategy = "";
            if (!Preset::SingBox::DomainStrategy.contains(outbound_domain_strategy)) outbound_domain_strategy = "";
        }
        _add(new configItem("direct_ip", &this->direct_ip, itemType::string));
        _add(new configItem("direct_domain", &this->direct_domain, itemType::string));
        _add(new configItem("proxy_ip", &this->proxy_ip, itemType::string));
        _add(new configItem("proxy_domain", &this->proxy_domain, itemType::string));
        _add(new configItem("block_ip", &this->block_ip, itemType::string));
        _add(new configItem("block_domain", &this->block_domain, itemType::string));
        _add(new configItem("def_outbound", &this->def_outbound, itemType::string));
        _add(new configItem("custom", &this->custom, itemType::string));
        //
        _add(new configItem("remote_dns", &this->remote_dns, itemType::string));
        _add(new configItem("remote_dns_strategy", &this->remote_dns_strategy, itemType::string));
        _add(new configItem("direct_dns", &this->direct_dns, itemType::string));
        _add(new configItem("direct_dns_strategy", &this->direct_dns_strategy, itemType::string));
        _add(new configItem("domain_strategy", &this->domain_strategy, itemType::string));
        _add(new configItem("outbound_domain_strategy", &this->outbound_domain_strategy, itemType::string));
        _add(new configItem("dns_routing", &this->dns_routing, itemType::boolean));
        _add(new configItem("sniffing_mode", &this->sniffing_mode, itemType::integer));
        _add(new configItem("use_dns_object", &this->use_dns_object, itemType::boolean));
        _add(new configItem("dns_object", &this->dns_object, itemType::string));
        _add(new configItem("dns_final_out", &this->dns_final_out, itemType::string));
    }

    QString Routing::DisplayRouting() const {
        return QString("[Proxy] %1\n[Proxy] %2\n[Direct] %3\n[Direct] %4\n[Block] %5\n[Block] %6\n[Default Outbound] %7\n[DNS] %8")
            .arg(SplitLinesSkipSharp(proxy_domain).join(","), 10)
            .arg(SplitLinesSkipSharp(proxy_ip).join(","), 10)
            .arg(SplitLinesSkipSharp(direct_domain).join(","), 10)
            .arg(SplitLinesSkipSharp(direct_ip).join(","), 10)
            .arg(SplitLinesSkipSharp(block_domain).join(","), 10)
            .arg(SplitLinesSkipSharp(block_ip).join(","), 10)
            .arg(def_outbound)
            .arg(use_dns_object ? "DNS Object" : "Simple DNS");
    }

    QStringList Routing::List() {
        QDir dr(ROUTES_PREFIX);
        return dr.entryList(QDir::Files);
    }

    bool Routing::SetToActive(const QString &name) {
        NekoGui::dataStore->routing = std::make_unique<Routing>();
        NekoGui::dataStore->routing->load_control_must = true;
        NekoGui::dataStore->routing->fn = ROUTES_PREFIX + name;
        auto ok = NekoGui::dataStore->routing->Load();
        if (ok) {
            NekoGui::dataStore->active_routing = name;
            NekoGui::dataStore->Save();
        }
        return ok;
    }

    // NO default extra core

    ExtraCore::ExtraCore() : JsonStore() {
        _add(new configItem("core_map", &this->core_map, itemType::string));
    }

    QString ExtraCore::Get(const QString &id) const {
        auto obj = QString2QJsonObject(core_map);
        for (const auto &c: obj.keys()) {
            if (c == id) return obj[id].toString();
        }
        return "";
    }

    void ExtraCore::Set(const QString &id, const QString &path) {
        auto obj = QString2QJsonObject(core_map);
        obj[id] = path;
        core_map = QJsonObject2QString(obj, true);
    }

    void ExtraCore::Delete(const QString &id) {
        auto obj = QString2QJsonObject(core_map);
        obj.remove(id);
        core_map = QJsonObject2QString(obj, true);
    }

    InboundAuthorization::InboundAuthorization() : JsonStore() {
        _add(new configItem("user", &this->username, itemType::string));
        _add(new configItem("pass", &this->password, itemType::string));
    }

    bool InboundAuthorization::NeedAuth() const {
        return !username.trimmed().isEmpty() && !password.trimmed().isEmpty();
    }

    // System Utils

    QString FindCoreAsset(const QString &name) {
        QStringList search{NekoGui::dataStore->v2ray_asset_dir};
        search << QApplication::applicationDirPath();
        search << "/usr/share/sing-geoip";
        search << "/usr/share/sing-geosite";
        search << "/usr/share/sing-box";
        search << "/usr/share/xray";
        search << "/usr/local/share/xray";
        search << "/opt/xray";
        search << "/usr/share/v2ray";
        search << "/usr/local/share/v2ray";
        search << "/opt/v2ray";
        for (const auto &dir: search) {
            if (dir.isEmpty()) continue;
            QFileInfo asset(dir + "/" + name);
            if (asset.exists()) {
                return asset.absoluteFilePath();
            }
        }
        return {};
    }

    QString FindNekoBoxCoreRealPath() {
        auto fn = QApplication::applicationDirPath() + "/nekobox_core";
        auto fi = QFileInfo(fn);
        if (fi.isSymLink()) return fi.symLinkTarget();
        return fn;
    }

    short isAdminCache = -1;

    // IsAdmin 主要判断：有无权限启动 Tun
    bool IsAdmin() {
        if (isAdminCache >= 0) return isAdminCache;

        bool admin = false;
#ifdef Q_OS_WIN
        admin = Windows_IsInAdmin();
#else
#ifdef Q_OS_LINUX
        admin |= Linux_GetCapString(FindNekoBoxCoreRealPath()).contains("cap_net_admin");
#endif
        admin |= geteuid() == 0;
#endif

        isAdminCache = admin;
        return admin;
    };

} // namespace NekoGui
