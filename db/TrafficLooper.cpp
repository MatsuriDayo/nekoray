#include "TrafficLooper.hpp"

#include "rpc/gRPC.h"
#include "ui/mainwindow.h"

#include <QThread>

namespace NekoRay::traffic {

    TrafficLooper *trafficLooper = new TrafficLooper;

    void TrafficLooper::update(TrafficData *item) {
#ifndef NKR_NO_GRPC
        auto uplink = NekoRay::rpc::defaultClient->QueryStats(item->tag, "uplink");
        auto downlink = NekoRay::rpc::defaultClient->QueryStats(item->tag, "downlink");

        item->downlink += downlink;
        item->uplink += uplink;

        //?
        item->downlink_rate = downlink * 1000 / dataStore->traffic_loop_interval;
        item->uplink_rate = uplink * 1000 / dataStore->traffic_loop_interval;
#endif
    }


    QJsonArray TrafficLooper::get_connection_list() {
#ifndef NKR_NO_GRPC
        auto str = NekoRay::rpc::defaultClient->ListV2rayConnections();
        QJsonDocument jsonDocument = QJsonDocument::fromJson(str.c_str());
        return jsonDocument.array();
#else
        return QJsonArray{};
#endif
    }

    [[noreturn]] void TrafficLooper::loop() {
        while (true) {
            auto sleep_ms = dataStore->traffic_loop_interval;
            auto user_disabled = sleep_ms == 0;
            if (sleep_ms < 500 || sleep_ms > 2000) sleep_ms = 1000;
            QThread::msleep(sleep_ms);
            if (user_disabled) continue;

            if (!loop_enabled) {
                // 停止
                if (looping) {
                    looping = false;
                    runOnUiThread([=] {
                        auto m = GetMainWindow();
                        m->refresh_status("STOP");
                    });
                }
                continue;
            } else {
                //开始
                if (!looping) {
                    looping = true;
                }
            }

            // do update
            loop_mutex.lock();

            for (const auto &item: items) {
                update(item.get());
            }
            update(bypass);

            // do conn list update
            auto conn_list = get_connection_list();

            loop_mutex.unlock();

            // post to UI
            runOnUiThread([=] {
                auto m = GetMainWindow();
                if (proxy != nullptr) {
                    m->refresh_status(
                            QObject::tr("Proxy: %1\nDirect: %2").arg(proxy->DisplaySpeed(), bypass->DisplaySpeed()));
                }
                for (const auto &item: items) {
                    if (item->id < 0) continue;
                    m->refresh_proxy_list(item->id);
                }
                m->refresh_connection_list(conn_list);
            });
        }
    }
}
