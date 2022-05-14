#include "main/NekoRay.hpp"
#include "db/Database.hpp"
#include "sub/RawUpdater.hpp"
#include "db/traffic/TrafficLooper.hpp"

namespace NekoRay {
    // 全局对象的定义
    DataStore *dataStore = new DataStore();
    ProfileManager *profileManager = new ProfileManager();

    namespace sub {
        RawUpdater *rawUpdater = new RawUpdater;
    }

    namespace traffic {
        TrafficLooper *trafficLooper = new TrafficLooper;
    }

    // default routing
    Routing::Routing(int preset) : JsonStore() {
        if (preset == 0) { // default conf with CN routing
            direct_ip = "geoip:cn\n"
                        "geoip:private";
            direct_domain = "geosite:cn";
            proxy_ip = "";
            proxy_domain = "";
            block_ip = "";
            block_domain = "geosite:category-ads-all\n"
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
                    object.insert(item->name, *(QString *) item->ptr);
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
                case stringList:
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
                if (debug_messagebox_verbose) {
                    MessageBoxWarning("提示",
                                      QString("unknown key\n%1\n%2").arg(key).arg(QJsonObject2QString(object, false)));
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
                        MessageBoxWarning("错误", "Not a int\n" + key);
                        continue;
                    }
                    *(long long *) item->ptr = value.toInt();
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
            if (debug_messagebox_verbose) MessageBoxWarning("QJsonParseError", error.errorString());
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
        file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
        file.write(save_content);
        file.close();

        return changed;
    }

    bool JsonStore::Load() {
        QFile file;
        file.setFileName(fn);

        if (!file.exists() && !load_control_force)
            return false;

        bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);
        if (!ok) {
            MessageBoxWarning("error", "can not open config " + fn + "\n" + file.errorString());
        } else {
            last_save_content = file.readAll();
            FromJsonBytes(last_save_content);
        }

        file.close();
        return ok;
    }

}
