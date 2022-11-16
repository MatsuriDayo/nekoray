#include "TrafficLooper.hpp"

#include "rpc/gRPC.h"
#include "ui/mainwindow_interface.h"

#include <QThread>

namespace NekoRay::traffic {

    TrafficLooper *trafficLooper = new TrafficLooper;
    QElapsedTimer trafficLooper_timer;

    std::unique_ptr<TrafficData> TrafficLooper::update_stats(TrafficData *item) {
#ifndef NKR_NO_GRPC
        // last update
        auto now = trafficLooper_timer.elapsed();
        auto interval = now - item->last_update;
        item->last_update = now;
        if (interval <= 0) return nullptr;

        // query
        auto uplink = NekoRay::rpc::defaultClient->QueryStats(item->tag, "uplink");
        auto downlink = NekoRay::rpc::defaultClient->QueryStats(item->tag, "downlink");

        // add diff
        item->downlink += downlink;
        item->uplink += uplink;
        item->downlink_rate = downlink * 1000 / interval;
        item->uplink_rate = uplink * 1000 / interval;

        // return diff
        auto ret = std::make_unique<TrafficData>(item->tag);
        ret->downlink = downlink;
        ret->uplink = uplink;
        ret->downlink_rate = item->downlink_rate;
        ret->uplink_rate = item->uplink_rate;
        return ret;
#endif
        return nullptr;
    }

    QJsonArray TrafficLooper::get_connection_list() {
#ifndef NKR_NO_GRPC
        auto str = NekoRay::rpc::defaultClient->ListConnections();
        QJsonDocument jsonDocument = QJsonDocument::fromJson(str.c_str());
        return jsonDocument.array();
#else
        return QJsonArray{};
#endif
    }

    void TrafficLooper::update_all() {
        std::map<std::string, std::unique_ptr<TrafficData>> updated; // tag to diff
        for (const auto &item: items) {
            auto data = item.get();
            auto diff = std::move(updated[data->tag]);
            // 避免重复查询一个 outbound tag
            if (diff == nullptr) {
                diff = update_stats(data);
                updated[data->tag] = std::move(diff);
            } else {
                data->uplink += diff->uplink;
                data->downlink += diff->downlink;
                data->uplink_rate = diff->uplink_rate;
                data->downlink_rate = diff->downlink_rate;
            }
        }
        update_stats(bypass);
    }

    void TrafficLooper::loop() {
        trafficLooper_timer.start();
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

            update_all();

            // do conn list update
            QJsonArray conn_list;
            if (dataStore->connection_statistics) {
                conn_list = get_connection_list();
            }

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
                if (dataStore->connection_statistics) {
                    m->refresh_connection_list(conn_list);
                }
            });
        }
    }

}
