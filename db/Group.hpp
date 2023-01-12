#pragma once

#include "main/NekoRay.hpp"
#include "ProxyEntity.hpp"

namespace NekoRay {
    class Group : public JsonStore {
    public:
        int id = -1;
        bool archive = false;
        QString name = "";
        QString url = "";
        QString info = "";
        qint64 last_update = 0;

        // list ui
        bool manually_column_width = false;
        QList<int> column_width;
        QList<int> order;

        Group();

        // 按 id 顺序
        [[nodiscard]] QList<QSharedPointer<ProxyEntity>> Profiles() const;

        // 按 显示 顺序
        [[nodiscard]] QList<QSharedPointer<ProxyEntity>> ProfilesWithOrder() const;
    };
} // namespace NekoRay
