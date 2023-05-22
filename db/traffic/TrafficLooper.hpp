#pragma once

#include <QString>
#include <QList>
#include <QMutex>

#include "TrafficData.hpp"

namespace NekoGui_traffic {
    class TrafficLooper {
    public:
        bool loop_enabled = false;
        bool looping = false;
        QMutex loop_mutex;

        QList<std::shared_ptr<TrafficData>> items;
        TrafficData *proxy = nullptr;

        void UpdateAll();

        void Loop();

    private:
        TrafficData *bypass = new TrafficData("bypass");

        [[nodiscard]] static TrafficData *update_stats(TrafficData *item);

        [[nodiscard]] static QJsonArray get_connection_list();
    };

    extern TrafficLooper *trafficLooper;
} // namespace NekoGui_traffic
