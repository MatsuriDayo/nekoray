#pragma once

#include "main/NekoGui.hpp"
#include "ProxyEntity.hpp"
#include "Group.hpp"

namespace NekoGui {
    class ProfileManager : public JsonStore {
    public:
        // Manager
        QMap<int, std::shared_ptr<ProxyEntity>> profiles;
        QMap<int, std::shared_ptr<Group>> groups;

        // JSON
        QList<int> _profiles;
        QList<int> _groups; // with order

        ProfileManager();

        [[nodiscard]] static std::shared_ptr<ProxyEntity> NewProxyEntity(const QString &type);

        [[nodiscard]] static std::shared_ptr<Group> NewGroup();

        bool AddProfile(const std::shared_ptr<ProxyEntity> &ent, int gid = -1);

        void DeleteProfile(int id);

        void MoveProfile(const std::shared_ptr<ProxyEntity> &ent, int gid);

        std::shared_ptr<ProxyEntity> GetProfile(int id);

        bool AddGroup(const std::shared_ptr<Group> &ent);

        void DeleteGroup(int gid);

        std::shared_ptr<Group> GetGroup(int id);

        std::shared_ptr<Group> CurrentGroup();

    private:
        void LoadManager();

        void SaveManager();

        [[nodiscard]] int NewProfileID() const;

        [[nodiscard]] int NewGroupID() const;

        static std::shared_ptr<ProxyEntity> LoadProxyEntity(const QString &jsonPath);

        static std::shared_ptr<Group> LoadGroup(const QString &jsonPath);
    };

    extern ProfileManager *profileManager;
} // namespace NekoGui
