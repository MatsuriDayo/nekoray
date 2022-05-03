#ifndef NEKORAY_TRAFFICLOOPER_HPP
#define NEKORAY_TRAFFICLOOPER_HPP

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

        void update(TrafficData *item);

        [[noreturn]] void loop();
    };

    extern TrafficLooper *trafficLooper;
}

#endif //NEKORAY_TRAFFICLOOPER_HPP
