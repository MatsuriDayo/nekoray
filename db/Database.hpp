#pragma once

#include "main/NekoRay.hpp"
#include "db/ProxyEntity.hpp"
#include "db/Group.hpp"

namespace NekoRay {
    class ProfileManager : public JsonStore {
    public:
        QMap<int, QSharedPointer<ProxyEntity>> profiles;
        QMap<int, QSharedPointer<Group>> groups;

        // JSON
        QList<int> _profiles;
        QList<int> _groups;

        ProfileManager() : JsonStore("groups/pm.json") {
            _hooks_after_load.push_back([=]() { LoadManager(); });
            _hooks_before_save.push_back([=]() { SaveManager(); });
            _add(new configItem("profiles", &_profiles, itemType::integerList));
            _add(new configItem("groups", &_groups, itemType::integerList));
        }

        [[nodiscard]] static QSharedPointer<ProxyEntity> NewProxyEntity(const QString &type);

        [[nodiscard]] static QSharedPointer<Group> NewGroup();

        bool AddProfile(const QSharedPointer<ProxyEntity> &ent, int gid = -1);

        void DeleteProfile(int id);

        QSharedPointer<ProxyEntity> GetProfile(int id);

        bool AddGroup(const QSharedPointer<Group> &ent);

        void DeleteGroup(int id);

        QSharedPointer<Group> GetGroup(int id);

        static QSharedPointer<Group> CurrentGroup();

        void AsyncUpdateSubscription(int gid);

    private:
        void LoadManager();

        void SaveManager();

        [[nodiscard]] int NewProfileID() const;

        [[nodiscard]] int NewGroupID() const;

        static QSharedPointer<ProxyEntity> LoadProxyEntity(const QString &jsonPath);

        static QSharedPointer<Group> LoadGroup(const QString &jsonPath);
    };

    extern ProfileManager *profileManager;
}
