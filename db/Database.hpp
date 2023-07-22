#pragma once

#include "main/NekoGui.hpp"
#include "ProxyEntity.hpp"
#include "Group.hpp"

namespace NekoGui {
    class ProfileManager : private JsonStore {
    public:
        // JsonStore

        // order -> id
        QList<int> groupsTabOrder;

        // Manager

        std::map<int, std::shared_ptr<ProxyEntity>> profiles;
        std::map<int, std::shared_ptr<Group>> groups;

        ProfileManager();

        // LoadManager Reset and loads profiles & groups
        void LoadManager();

        void SaveManager();

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
        // sort by id
        QList<int> profilesIdOrder;
        QList<int> groupsIdOrder;

        [[nodiscard]] int NewProfileID() const;

        [[nodiscard]] int NewGroupID() const;

        static std::shared_ptr<ProxyEntity> LoadProxyEntity(const QString &jsonPath);

        static std::shared_ptr<Group> LoadGroup(const QString &jsonPath);
    };

    extern ProfileManager *profileManager;
} // namespace NekoGui
