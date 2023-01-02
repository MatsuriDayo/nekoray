#pragma once

#include "main/NekoRay.hpp"
#include "ProxyEntity.hpp"
#include "Group.hpp"

namespace NekoRay {
    class ProfileManager : public JsonStore {
    public:
        // Manager
        QMap<int, QSharedPointer<ProxyEntity>> profiles;
        QMap<int, QSharedPointer<Group>> groups;

        // JSON
        QList<int> _profiles;
        QList<int> _groups; // with order

        ProfileManager();

        [[nodiscard]] static QSharedPointer<ProxyEntity> NewProxyEntity(const QString &type);

        [[nodiscard]] static QSharedPointer<Group> NewGroup();

        bool AddProfile(const QSharedPointer<ProxyEntity> &ent, int gid = -1);

        void DeleteProfile(int id);

        void MoveProfile(const QSharedPointer<ProxyEntity> &ent, int gid);

        QSharedPointer<ProxyEntity> GetProfile(int id);

        bool AddGroup(const QSharedPointer<Group> &ent);

        void DeleteGroup(int gid);

        QSharedPointer<Group> GetGroup(int id);

        QSharedPointer<Group> CurrentGroup();

    private:
        void LoadManager();

        void SaveManager();

        [[nodiscard]] int NewProfileID() const;

        [[nodiscard]] int NewGroupID() const;

        static QSharedPointer<ProxyEntity> LoadProxyEntity(const QString &jsonPath);

        static QSharedPointer<Group> LoadGroup(const QString &jsonPath);
    };

    extern ProfileManager *profileManager;
} // namespace NekoRay
