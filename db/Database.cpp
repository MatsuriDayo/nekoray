#include "Database.hpp"

#include "fmt/includes.h"

#include <QFile>
#include <QDir>
#include <QColor>

namespace NekoGui {

    ProfileManager *profileManager = new ProfileManager();

    ProfileManager::ProfileManager() : JsonStore("groups/pm.json") {
        _add(new configItem("groups", &groupsTabOrder, itemType::integerList));
    }

    QList<int> filterIntJsonFile(const QString &path) {
        QList<int> result;
        QDir dr(path);
        auto entryList = dr.entryList(QDir::Files);
        for (auto e: entryList) {
            e = e.toLower();
            if (!e.endsWith(".json", Qt::CaseInsensitive)) continue;
            e = e.remove(".json", Qt::CaseInsensitive);
            bool ok;
            auto id = e.toInt(&ok);
            if (ok) {
                result << id;
            }
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    void ProfileManager::LoadManager() {
        JsonStore::Load();
        //
        profiles = {};
        groups = {};
        profilesIdOrder = filterIntJsonFile("profiles");
        groupsIdOrder = filterIntJsonFile("groups");
        // Load Proxys
        QList<int> delProfile;
        for (auto id: profilesIdOrder) {
            auto ent = LoadProxyEntity(QString("profiles/%1.json").arg(id));
            // Corrupted profile?
            if (ent == nullptr || ent->bean == nullptr || ent->bean->version == -114514) {
                delProfile << id;
                continue;
            }
            profiles[id] = ent;
        }
        // Clear Corrupted profile
        for (auto id: delProfile) {
            DeleteProfile(id);
        }
        // Load Groups
        auto loadedOrder = groupsTabOrder;
        groupsTabOrder = {};
        for (auto id: groupsIdOrder) {
            auto ent = LoadGroup(QString("groups/%1.json").arg(id));
            // Corrupted group?
            if (ent->id != id) {
                continue;
            }
            // Ensure order contains every group
            if (!loadedOrder.contains(id)) {
                loadedOrder << id;
            }
            groups[id] = ent;
        }
        // Ensure groups contains order
        for (auto id: loadedOrder) {
            if (groups.count(id)) {
                groupsTabOrder << id;
            }
        }
        // First setup
        if (groups.empty()) {
            auto defaultGroup = NekoGui::ProfileManager::NewGroup();
            defaultGroup->name = QObject::tr("Default");
            NekoGui::profileManager->AddGroup(defaultGroup);
        }
        //
        if (dataStore->flag_reorder) {
            {
                // remove all (contains orphan)
                for (const auto &profile: profiles) {
                    QFile::remove(profile.second->fn);
                }
            }
            std::map<int, int> gidOld2New;
            {
                int i = 0;
                int ii = 0;
                QList<int> newProfilesIdOrder;
                std::map<int, std::shared_ptr<ProxyEntity>> newProfiles;
                for (auto gid: groupsTabOrder) {
                    auto group = GetGroup(gid);
                    gidOld2New[gid] = ii++;
                    for (auto const &profile: group->ProfilesWithOrder()) {
                        auto oldId = profile->id;
                        auto newId = i++;
                        profile->id = newId;
                        profile->gid = gidOld2New[gid];
                        profile->fn = QString("profiles/%1.json").arg(newId);
                        profile->Save();
                        newProfiles[newId] = profile;
                        newProfilesIdOrder << newId;
                    }
                    group->order = {};
                    group->Save();
                }
                profiles = newProfiles;
                profilesIdOrder = newProfilesIdOrder;
            }
            {
                QList<int> newGroupsIdOrder;
                std::map<int, std::shared_ptr<Group>> newGroups;
                for (auto oldGid: groupsTabOrder) {
                    auto newId = gidOld2New[oldGid];
                    auto group = groups[oldGid];
                    QFile::remove(group->fn);
                    group->id = newId;
                    group->fn = QString("groups/%1.json").arg(newId);
                    group->Save();
                    newGroups[newId] = group;
                    newGroupsIdOrder << newId;
                }
                groups = newGroups;
                groupsIdOrder = newGroupsIdOrder;
                groupsTabOrder = newGroupsIdOrder;
            }
            MessageBoxInfo(software_name, "Profiles and groups reorder complete.");
        }
    }

    void ProfileManager::SaveManager() {
        JsonStore::Save();
    }

    std::shared_ptr<ProxyEntity> ProfileManager::LoadProxyEntity(const QString &jsonPath) {
        // Load type
        ProxyEntity ent0(nullptr, nullptr);
        ent0.fn = jsonPath;
        auto validJson = ent0.Load();
        auto type = ent0.type;

        // Load content
        std::shared_ptr<ProxyEntity> ent;
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

    std::shared_ptr<ProxyEntity> ProfileManager::NewProxyEntity(const QString &type) {
        NekoGui_fmt::AbstractBean *bean;

        if (type == "socks") {
            bean = new NekoGui_fmt::SocksHttpBean(NekoGui_fmt::SocksHttpBean::type_Socks5);
        } else if (type == "http") {
            bean = new NekoGui_fmt::SocksHttpBean(NekoGui_fmt::SocksHttpBean::type_HTTP);
        } else if (type == "shadowsocks") {
            bean = new NekoGui_fmt::ShadowSocksBean();
        } else if (type == "chain") {
            bean = new NekoGui_fmt::ChainBean();
        } else if (type == "vmess") {
            bean = new NekoGui_fmt::VMessBean();
        } else if (type == "trojan") {
            bean = new NekoGui_fmt::TrojanVLESSBean(NekoGui_fmt::TrojanVLESSBean::proxy_Trojan);
        } else if (type == "vless") {
            bean = new NekoGui_fmt::TrojanVLESSBean(NekoGui_fmt::TrojanVLESSBean::proxy_VLESS);
        } else if (type == "naive") {
            bean = new NekoGui_fmt::NaiveBean();
        } else if (type == "hysteria") {
            bean = new NekoGui_fmt::QUICBean(NekoGui_fmt::QUICBean::proxy_Hysteria);
        } else if (type == "hysteria2") {
            bean = new NekoGui_fmt::QUICBean(NekoGui_fmt::QUICBean::proxy_Hysteria2);
        } else if (type == "tuic") {
            bean = new NekoGui_fmt::QUICBean(NekoGui_fmt::QUICBean::proxy_TUIC);
        } else if (type == "custom") {
            bean = new NekoGui_fmt::CustomBean();
        } else {
            bean = new NekoGui_fmt::AbstractBean(-114514);
        }

        auto ent = std::make_shared<ProxyEntity>(bean, type);
        return ent;
    }

    std::shared_ptr<Group> ProfileManager::NewGroup() {
        auto ent = std::make_shared<Group>();
        return ent;
    }

    // ProxyEntity

    ProxyEntity::ProxyEntity(NekoGui_fmt::AbstractBean *bean, const QString &type_) {
        if (type_ != nullptr) this->type = type_;

        _add(new configItem("type", &type, itemType::string));
        _add(new configItem("id", &id, itemType::integer));
        _add(new configItem("gid", &gid, itemType::integer));
        _add(new configItem("yc", &latency, itemType::integer));
        _add(new configItem("report", &full_test_report, itemType::string));

        // 可以不关联 bean，只加载 ProxyEntity 的信息
        if (bean != nullptr) {
            this->bean = std::shared_ptr<NekoGui_fmt::AbstractBean>(bean);
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
            auto greenMs = dataStore->test_latency_url.startsWith("https://") ? 200 : 100;
            if (latency < greenMs) {
                return Qt::darkGreen;
            } else {
                return Qt::darkYellow;
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
            return profilesIdOrder.last() + 1;
        }
    }

    bool ProfileManager::AddProfile(const std::shared_ptr<ProxyEntity> &ent, int gid) {
        if (ent->id >= 0) {
            return false;
        }

        ent->gid = gid < 0 ? dataStore->current_group : gid;
        ent->id = NewProfileID();
        profiles[ent->id] = ent;
        profilesIdOrder.push_back(ent->id);

        ent->fn = QString("profiles/%1.json").arg(ent->id);
        ent->Save();
        return true;
    }

    void ProfileManager::DeleteProfile(int id) {
        if (id < 0) return;
        if (dataStore->started_id == id) return;
        profiles.erase(id);
        profilesIdOrder.removeAll(id);
        QFile(QString("profiles/%1.json").arg(id)).remove();
    }

    void ProfileManager::MoveProfile(const std::shared_ptr<ProxyEntity> &ent, int gid) {
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

    std::shared_ptr<ProxyEntity> ProfileManager::GetProfile(int id) {
        return profiles.count(id) ? profiles[id] : nullptr;
    }

    // Group

    Group::Group() {
        _add(new configItem("id", &id, itemType::integer));
        _add(new configItem("front_proxy_id", &front_proxy_id, itemType::integer));
        _add(new configItem("archive", &archive, itemType::boolean));
        _add(new configItem("skip_auto_update", &skip_auto_update, itemType::boolean));
        _add(new configItem("name", &name, itemType::string));
        _add(new configItem("order", &order, itemType::integerList));
        _add(new configItem("url", &url, itemType::string));
        _add(new configItem("info", &info, itemType::string));
        _add(new configItem("lastup", &sub_last_update, itemType::integer64));
        _add(new configItem("manually_column_width", &manually_column_width, itemType::boolean));
        _add(new configItem("column_width", &column_width, itemType::integerList));
    }

    std::shared_ptr<Group> ProfileManager::LoadGroup(const QString &jsonPath) {
        auto ent = std::make_shared<Group>();
        ent->fn = jsonPath;
        ent->Load();
        return ent;
    }

    int ProfileManager::NewGroupID() const {
        if (groups.empty()) {
            return 0;
        } else {
            return groupsIdOrder.last() + 1;
        }
    }

    bool ProfileManager::AddGroup(const std::shared_ptr<Group> &ent) {
        if (ent->id >= 0) {
            return false;
        }

        ent->id = NewGroupID();
        groups[ent->id] = ent;
        groupsIdOrder.push_back(ent->id);
        groupsTabOrder.push_back(ent->id);

        ent->fn = QString("groups/%1.json").arg(ent->id);
        ent->Save();
        return true;
    }

    void ProfileManager::DeleteGroup(int gid) {
        if (groups.size() <= 1) return;
        QList<int> toDelete;
        for (const auto &[id, profile]: profiles) {
            if (profile->gid == gid) toDelete += id; // map访问中，不能操作
        }
        for (const auto &id: toDelete) {
            DeleteProfile(id);
        }
        groups.erase(gid);
        groupsIdOrder.removeAll(gid);
        groupsTabOrder.removeAll(gid);
        QFile(QString("groups/%1.json").arg(gid)).remove();
    }

    std::shared_ptr<Group> ProfileManager::GetGroup(int id) {
        return groups.count(id) ? groups[id] : nullptr;
    }

    std::shared_ptr<Group> ProfileManager::CurrentGroup() {
        return GetGroup(dataStore->current_group);
    }

    QList<std::shared_ptr<ProxyEntity>> Group::Profiles() const {
        QList<std::shared_ptr<ProxyEntity>> ret;
        for (const auto &[_, profile]: profileManager->profiles) {
            if (id == profile->gid) ret += profile;
        }
        return ret;
    }

    QList<std::shared_ptr<ProxyEntity>> Group::ProfilesWithOrder() const {
        if (order.isEmpty()) {
            return Profiles();
        } else {
            QList<std::shared_ptr<ProxyEntity>> ret;
            for (auto _id: order) {
                auto ent = profileManager->GetProfile(_id);
                if (ent != nullptr) ret += ent;
            }
            return ret;
        }
    }

} // namespace NekoGui