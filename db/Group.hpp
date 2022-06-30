#pragma once

#include "main/NekoRay.hpp"
#include "ProxyEntity.hpp"

namespace NekoRay {
    class Group : public JsonStore {
    public:
        int id = -1;
        bool archive = false;
        QString name = "";
        QList<int> order;
        QString url = "";
        QString info = "";

        Group();

        // 按 id 顺序
        [[nodiscard]] QList<QSharedPointer<ProxyEntity>> Profiles() const;
    };
}
