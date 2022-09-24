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
        TrafficData *bypass = new TrafficData("bypass");
        TrafficData *proxy = nullptr;

        static std::unique_ptr<TrafficData> update_stats(TrafficData *item);

        static QJsonArray get_connection_list();

        void update_all();

        void loop();
    };

    extern TrafficLooper *trafficLooper;
}
