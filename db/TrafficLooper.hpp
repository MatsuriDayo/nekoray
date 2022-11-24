#pragma once

#include <QSharedPointer>
#include <QString>
#include <QList>
#include <QMutex>

#include "TrafficData.hpp"

namespace NekoRay::traffic {
    class TrafficLooper {
    public:
        bool loop_enabled = false;
        bool looping = false;
        QMutex loop_mutex;

        QList<QSharedPointer<TrafficData>> items;
        TrafficData *proxy = nullptr;

        void UpdateAll();

        void Loop();

    private:
        TrafficData *bypass = new TrafficData("bypass");

        [[nodiscard]] static TrafficData *update_stats(TrafficData *item);

        [[nodiscard]] static QJsonArray get_connection_list();
    };

    extern TrafficLooper *trafficLooper;
} // namespace NekoRay::traffic
