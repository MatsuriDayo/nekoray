#include "NekoRay.hpp"

#include <QFile>
#include <QDir>
#include <QApplication>
#include <QFileInfo>
#include <QSharedPointer>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace NekoRay {

    DataStore *dataStore = new DataStore();

    // datastore

    DataStore::DataStore() : JsonStore() {
        _add(new configItem("extraCore", dynamic_cast<JsonStore *>(extraCore), itemType::jsonStore));

        _add(new configItem("user_agent", &user_agent, itemType::string));
        _add(new configItem("test_url", &test_url, itemType::string));
        _add(new configItem("current_group", &current_group, itemType::integer));
        _add(new configItem("inbound_address", &inbound_address, itemType::string));
        _add(new configItem("inbound_socks_port", &inbound_socks_port, itemType::integer));
        _add(new configItem("inbound_http_port", &inbound_http_port, itemType::integer));
        _add(new configItem("log_level", &log_level, itemType::string));
        _add(new configItem("remote_dns", &remote_dns, itemType::string));
        _add(new configItem("direct_dns", &direct_dns, itemType::string));
        _add(new configItem("domain_matcher", &domain_matcher, itemType::integer));
        _add(new configItem("domain_strategy", &domain_strategy, itemType::string));
        _add(new configItem("outbound_domain_strategy", &outbound_domain_strategy, itemType::string));
        _add(new configItem("sniffing_mode", &sniffing_mode, itemType::integer));
        _add(new configItem("mux_cool", &mux_cool, itemType::integer));
        _add(new configItem("traffic_loop_interval", &traffic_loop_interval, itemType::integer));
        _add(new configItem("dns_routing", &dns_routing, itemType::boolean));
        _add(new configItem("test_concurrent", &test_concurrent, itemType::integer));
        _add(new configItem("theme", &theme, itemType::string));
        _add(new configItem("custom_inbound", &custom_inbound, itemType::string));
        _add(new configItem("custom_route", &custom_route_global, itemType::string));
        _add(new configItem("v2ray_asset_dir", &v2ray_asset_dir, itemType::string));
        _add(new configItem("sub_use_proxy", &sub_use_proxy, itemType::boolean));
        _add(new configItem("enhance_domain", &enhance_resolve_server_domain, itemType::boolean));
        _add(new configItem("remember_id", &remember_id, itemType::integer));
        _add(new configItem("remember_enable", &remember_enable, itemType::boolean));
        _add(new configItem("language", &language, itemType::integer));
        _add(new configItem("spmode", &remember_spmode, itemType::integer));
        _add(new configItem("insecure_hint", &insecure_hint, itemType::boolean));
        _add(new configItem("skip_cert", &skip_cert, itemType::boolean));
        _add(new configItem("hk_mw", &hotkey_mainwindow, itemType::string));
        _add(new configItem("hk_group", &hotkey_group, itemType::string));
        _add(new configItem("hk_route", &hotkey_route, itemType::string));
        _add(new configItem("fakedns", &fake_dns, itemType::boolean));
        _add(new configItem("active_routing", &active_routing, itemType::string));
        _add(new configItem("mw_size", &mw_size, itemType::string));
        _add(new configItem("conn_stat", &connection_statistics, itemType::boolean));
        _add(new configItem("vpn_impl", &vpn_implementation, itemType::integer));
        _add(new configItem("vpn_mtu", &vpn_mtu, itemType::integer));
        _add(new configItem("vpn_ipv6", &vpn_ipv6, itemType::boolean));
        _add(new configItem("vpn_hide_console", &vpn_hide_console, itemType::boolean));
        _add(new configItem("vpn_strict_route", &vpn_strict_route, itemType::boolean));
        _add(new configItem("vpn_bypass_process", &vpn_bypass_process, itemType::string));
        _add(new configItem("vpn_bypass_cidr", &vpn_bypass_cidr, itemType::string));
        _add(new configItem("check_include_pre", &check_include_pre, itemType::boolean));
        _add(new configItem("sp_format", &system_proxy_format, itemType::string));
        _add(new configItem("sub_clear", &sub_clear, itemType::boolean));
        _add(new configItem("sub_insecure", &sub_insecure, itemType::boolean));
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
                "domain:app-measurement.com\n"
                "domain:firebase.io\n"
                "domain:crashlytics.com\n"
                "domain:google-analytics.com";
        }
        _add(new configItem("direct_ip", &this->direct_ip, itemType::string));
        _add(new configItem("direct_domain", &this->direct_domain, itemType::string));
        _add(new configItem("proxy_ip", &this->proxy_ip, itemType::string));
        _add(new configItem("proxy_domain", &this->proxy_domain, itemType::string));
        _add(new configItem("block_ip", &this->block_ip, itemType::string));
        _add(new configItem("block_domain", &this->block_domain, itemType::string));
        _add(new configItem("custom", &this->custom, itemType::string));
    }

    QString Routing::toString() const {
        return QString("[Proxy] %1\n[Proxy] %2\n[Direct] %3\n[Direct] %4\n[Block] %5\n[Block] %6")
            .arg(SplitLines(proxy_domain).join(","))
            .arg(SplitLines(proxy_ip).join(","))
            .arg(SplitLines(direct_domain).join(","))
            .arg(SplitLines(direct_ip).join(","))
            .arg(SplitLines(block_domain).join(","))
            .arg(SplitLines(block_ip).join(","));
    }

    QStringList Routing::List() {
        QStringList l;
        QDir d;
        if (d.exists(ROUTES_PREFIX)) {
            QDir dr(ROUTES_PREFIX);
            return dr.entryList(QDir::Files);
        }
        return l;
    }

    void Routing::SetToActive(const QString &name) {
        dataStore->routing->fn = ROUTES_PREFIX + name;
        dataStore->routing->Load();
        dataStore->active_routing = name;
        dataStore->Save();
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

    // 添加关联
    void JsonStore::_add(configItem *item) {
        _map.insert(item->name, QSharedPointer<configItem>(item));
    }

    QSharedPointer<configItem> JsonStore::_get(const QString &name) {
        // 直接 [] 会设置一个 nullptr ，所以先判断是否存在
        if (_map.contains(name)) {
            return _map[name];
        }
        return nullptr;
    }

    QJsonObject JsonStore::ToJson() {
        QJsonObject object;
        for (const auto &_item: _map) {
            auto item = _item.get();
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
                if (debug_verbose) {
                    qDebug() << QString("unknown key\n%1\n%2").arg(key, QJsonObject2QString(object, false));
                }
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
                        MessageBoxWarning("错误", "Not a string\n" + key);
                        continue;
                    }
                    *(QString *) item->ptr = value.toString();
                    break;
                case itemType::integer:
                    if (value.type() != QJsonValue::Double) {
                        MessageBoxWarning("错误", "Not a int\n" + key);
                        continue;
                    }
                    *(int *) item->ptr = value.toInt();
                    break;
                case itemType::integer64:
                    if (value.type() != QJsonValue::Double) {
                        MessageBoxWarning("错误", "Not a int64\n" + key);
                        continue;
                    }
                    *(long long *) item->ptr = value.toDouble();
                    break;
                case itemType::boolean:
                    if (value.type() != QJsonValue::Bool) {
                        MessageBoxWarning("错误", "Not a bool\n" + key);
                        continue;
                    }
                    *(bool *) item->ptr = value.toBool();
                    break;
                case itemType::stringList:
                    if (value.type() != QJsonValue::Array) {
                        MessageBoxWarning("错误", "Not a Array\n" + key);
                        continue;
                    }
                    *(QList<QString> *) item->ptr = QJsonArray2QListString(value.toArray());
                    break;
                case itemType::integerList:
                    if (value.type() != QJsonValue::Array) {
                        MessageBoxWarning("错误", "Not a Array\n" + key);
                        continue;
                    }
                    *(QList<int> *) item->ptr = QJsonArray2QListInt(value.toArray());
                    break;
                case itemType::jsonStore:
                    if (value.type() != QJsonValue::Object) {
                        MessageBoxWarning("错误", "Not a json object\n" + key);
                        continue;
                    }
                    if (load_control_no_jsonStore)
                        continue;
                    ((JsonStore *) item->ptr)->FromJson(value.toObject());
                    break;
            }
        }

        for (const auto &hook: _hooks_after_load) {
            hook();
        }
    }

    void JsonStore::FromJsonBytes(const QByteArray &data) {
        QJsonParseError error{};
        auto document = QJsonDocument::fromJson(data, &error);

        if (error.error != error.NoError) {
            if (debug_verbose) qDebug() << "QJsonParseError" << error.errorString();
            return;
        }

        FromJson(document.object());
    }

    bool JsonStore::Save() {
        for (const auto &hook: _hooks_before_save) {
            hook();
        }

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

        if (!file.exists() && !load_control_force)
            return false;

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

    //

    QString FindCoreAsset(const QString &name) {
        QStringList search{dataStore->v2ray_asset_dir};
        search << QApplication::applicationDirPath();
        search << "/usr/share/v2ray";
        search << "/usr/local/share/v2ray";
        search << "/opt/v2ray";
        search << "/usr/share/xray";
        search << "/usr/local/share/xray";
        search << "/opt/xray";
        search << "/usr/share/sing-geoip";
        search << "/usr/share/sing-geosite";
        for (const auto &dir: search) {
            if (dir.isEmpty()) continue;
            QFileInfo asset(dir + "/" + name);
            if (asset.exists()) {
                return asset.absoluteFilePath();
            }
        }
        return {};
    }

} // namespace NekoRay
