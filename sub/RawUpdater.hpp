#pragma once

#include <QString>

namespace NekoRay::sub {
    class RawUpdater {
    private:
        void updateClash(const QString &str);

        void update(const QString &str);

        int update_sub_gid = -1; // 订阅更新标志，订阅更新前设置
    public:
        void AsyncUpdate(const QString &str, int _update_sub_gid = -1);
    };

    extern RawUpdater *rawUpdater;
}

