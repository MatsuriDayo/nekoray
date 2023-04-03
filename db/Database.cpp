#include "Database.hpp"

#include "fmt/includes.h"

#include <QFile>
#include <QColor>

namespace NekoRay {

    ProfileManager *profileManager = new ProfileManager();

    ProfileManager::ProfileManager() : JsonStore("groups/pm.json") {
        callback_after_load = [this]() { LoadManager(); };
        callback_before_save = [this]() { SaveManager(); };
        _add(new configItem("profiles", &_profiles, itemType::integerList));
        _add(new configItem("groups", &_groups, itemType::integerList));
    }

    void ProfileManager::LoadManager() {
        profiles = {};
        groups = {};
        QList<int> invalidProfileId;
        for (auto id: _profiles) {
            auto ent = LoadProxyEntity(QString("profiles/%1.json").arg(id));
            if (ent == nullptr || ent->bean == nullptr || ent->bean->version == -114514) {
                // clear invalid profile
                invalidProfileId << id;
                continue;
            }
            profiles[id] = ent;
        }
        for (auto id: _groups) {
            groups[id] = LoadGroup(QString("groups/%1.json").arg(id));
        }
        for (auto id: invalidProfileId) {
            DeleteProfile(id);
        }
    }

    void ProfileManager::SaveManager() {
    }

    QSharedPointer<ProxyEntity> ProfileManager::LoadProxyEntity(const QString &jsonPath) {
        // Load type
        ProxyEntity ent0(nullptr, nullptr);
        ent0.fn = jsonPath;
        auto validJson = ent0.Load();
        auto type = ent0.type;

        // Load content
        QSharedPointer<ProxyEntity> ent;
        bool validType = validJson;

        if (validType) {
            ent = NewProxyEntity(type);
            validType = ent->bean->version != -114514;
        }

        if (validType) {
            ent->load_control_must = true;
            ent->fn = jsonPath;
            ent->Load();
        }
        return ent;
    }

    //  新建的不给 fn 和 id

    QSharedPointer<ProxyEntity> ProfileManager::NewProxyEntity(const QString &type) {
        fmt::AbstractBean *bean;

        if (type == "socks") {
            bean = new fmt::SocksHttpBean(NekoRay::fmt::SocksHttpBean::type_Socks5);
        } else if (type == "http") {
            bean = new fmt::SocksHttpBean(NekoRay::fmt::SocksHttpBean::type_HTTP);
        } else if (type == "shadowsocks") {
            bean = new fmt::ShadowSocksBean();
        } else if (type == "chain") {
            bean = new fmt::ChainBean();
        } else if (type == "vmess") {
            bean = new fmt::VMessBean();
        } else if (type == "trojan") {
            bean = new fmt::TrojanVLESSBean(fmt::TrojanVLESSBean::proxy_Trojan);
        } else if (type == "vless") {
            bean = new fmt::TrojanVLESSBean(fmt::TrojanVLESSBean::proxy_VLESS);
        } else if (type == "naive") {
            bean = new fmt::NaiveBean();
        } else if (type == "custom") {
            bean = new fmt::CustomBean();
        } else {
            bean = new fmt::AbstractBean(-114514);
        }

        auto ent = QSharedPointer<ProxyEntity>(new ProxyEntity(bean, type));
        return ent;
    }

    QSharedPointer<Group> ProfileManager::NewGroup() {
        auto ent = QSharedPointer<Group>(new Group());
        return ent;
    }

    // ProxyEntity

    ProxyEntity::ProxyEntity(fmt::AbstractBean *bean, const QString &type_) {
        if (type_ != nullptr) this->type = type_;

        _add(new configItem("type", &type, itemType::string));
        _add(new configItem("id", &id, itemType::integer));
        _add(new configItem("gid", &gid, itemType::integer));
        _add(new configItem("yc", &latency, itemType::integer));
        _add(new configItem("report", &full_test_report, itemType::string));

        // 可以不关联 bean，只加载 ProxyEntity 的信息
        if (bean != nullptr) {
            this->bean = QSharedPointer<fmt::AbstractBean>(bean);
            // 有虚函数就要在这里 dynamic_cast
            _add(new configItem("bean", dynamic_cast<JsonStore *>(bean), itemType::jsonStore));
            _add(new configItem("traffic", dynamic_cast<JsonStore *>(traffic_data.get()), itemType::jsonStore));
        }
    };

    QString ProxyEntity::DisplayLatency() const {
        if (latency < 0) {
            return QObject::tr("Unavailable");
        } else if (latency > 0) {
            return UNICODE_LRO + QString("%1 ms").arg(latency);
        } else {
            return "";
        }
    }

    QColor ProxyEntity::DisplayLatencyColor() const {
        if (latency < 0) {
            return Qt::red;
        } else if (latency > 0) {
            if (latency < 100) {
                return Qt::darkGreen;
            } else if (latency < 200) {
                return Qt::darkYellow;
            } else {
                return Qt::red;
            }
        } else {
            return {};
        }
    }

    // Profile

    int ProfileManager::NewProfileID() const {
        if (profiles.empty()) {
            return 0;
        } else {
            return profiles.lastKey() + 1;
        }
    }

    bool ProfileManager::AddProfile(const QSharedPointer<ProxyEntity> &ent, int gid) {
        if (ent->id >= 0) {
            return false;
        }

        ent->gid = gid < 0 ? dataStore->current_group : gid;
        ent->id = NewProfileID();
        profiles[ent->id] = ent;
        _profiles.push_back(ent->id);
        Save();

        ent->fn = QString("profiles/%1.json").arg(ent->id);
        ent->Save();
        return true;
    }

    void ProfileManager::DeleteProfile(int id) {
        if (id < 0) return;
        if (dataStore->started_id == id) return;
        profiles.remove(id);
        _profiles.removeAll(id);
        Save();
        QFile(QString("profiles/%1.json").arg(id)).remove();
    }

    void ProfileManager::MoveProfile(const QSharedPointer<ProxyEntity> &ent, int gid) {
        if (gid == ent->gid || gid < 0) return;
        auto oldGroup = GetGroup(ent->gid);
        if (oldGroup != nullptr && !oldGroup->order.isEmpty()) {
            oldGroup->order.removeAll(ent->id);
            oldGroup->Save();
        }
        auto newGroup = GetGroup(gid);
        if (newGroup != nullptr && !newGroup->order.isEmpty()) {
            newGroup->order.push_back(ent->id);
            newGroup->Save();
        }
        ent->gid = gid;
        ent->Save();
    }

    QSharedPointer<ProxyEntity> ProfileManager::GetProfile(int id) {
        return profiles.value(id, nullptr);
    }

    // Group

    Group::Group() {
        _add(new configItem("id", &id, itemType::integer));
        _add(new configItem("archive", &archive, itemType::boolean));
        _add(new configItem("name", &name, itemType::string));
        _add(new configItem("order", &order, itemType::integerList));
        _add(new configItem("url", &url, itemType::string));
        _add(new configItem("info", &info, itemType::string));
        _add(new configItem("lastup", &last_update, itemType::integer64));
        _add(new configItem("manually_column_width", &manually_column_width, itemType::boolean));
        _add(new configItem("column_width", &column_width, itemType::integerList));
    }

    QSharedPointer<Group> ProfileManager::LoadGroup(const QString &jsonPath) {
        QSharedPointer<Group> ent = QSharedPointer<Group>(new Group());
        ent->fn = jsonPath;
        ent->Load();
        return ent;
    }

    int ProfileManager::NewGroupID() const {
        if (groups.empty()) {
            return 0;
        } else {
            return groups.lastKey() + 1;
        }
    }

    bool ProfileManager::AddGroup(const QSharedPointer<Group> &ent) {
        if (ent->id >= 0) {
            return false;
        }

        ent->id = NewGroupID();
        groups[ent->id] = ent;
        _groups.push_back(ent->id);
        Save();

        ent->fn = QString("groups/%1.json").arg(ent->id);
        ent->Save();
        return true;
    }

    void ProfileManager::DeleteGroup(int gid) {
        if (groups.count() == 1) return;
        QList<int> toDelete;
        for (const auto &profile: profiles) {
            if (profile->gid == gid) toDelete += profile->id; // map访问中，不能操作
        }
        for (const auto &id: toDelete) {
            DeleteProfile(id);
        }
        groups.remove(gid);
        _groups.removeAll(gid);
        Save();
        QFile(QString("groups/%1.json").arg(gid)).remove();
    }

    QSharedPointer<Group> ProfileManager::GetGroup(int id) {
        return groups.value(id, nullptr);
    }

    QSharedPointer<Group> ProfileManager::CurrentGroup() {
        return GetGroup(NekoRay::dataStore->current_group);
    }

    QList<QSharedPointer<ProxyEntity>> Group::Profiles() const {
        QList<QSharedPointer<ProxyEntity>> ret;
        for (const auto &ent: profileManager->profiles) {
            if (id == ent->gid) ret += ent;
        }
        return ret;
    }

    QList<QSharedPointer<ProxyEntity>> Group::ProfilesWithOrder() const {
        if (order.isEmpty()) {
            return Profiles();
        } else {
            QList<QSharedPointer<ProxyEntity>> ret;
            for (auto _id: order) {
                auto ent = profileManager->GetProfile(_id);
                if (ent != nullptr) ret += ent;
            }
            return ret;
        }
    }

} // namespace NekoRay