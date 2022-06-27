#pragma once

#include <QString>

namespace NekoRay::sub {
    class RawUpdater {
    public:
        void updateClash(const QString &str);

        void update(const QString &str);

        int gid_add_to = -1; // 导入到指定组 -1 为当前选中组

        int update_counter = 0; // 新增了多少个配置
    };

    class GroupUpdater : public QObject {
    Q_OBJECT

    public:
        void AsyncUpdate(const QString &str, int _sub_gid = -1,
                         QObject *caller = nullptr, const std::function<void()> &callback = nullptr);

        void Update(const QString &_str, int _sub_gid = -1, bool _not_sub_as_url = false);

    signals:

        void AsyncUpdateCallback(QObject *caller);
    };

    extern GroupUpdater *groupUpdater;
}

