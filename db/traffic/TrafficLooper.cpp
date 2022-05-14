#include "TrafficLooper.hpp"
#include "rpc/gRPC.h"

#include <QThread>

#include "NekoRay.hpp"
#include "ui/mainwindow_message.h"

namespace NekoRay::traffic {
    void TrafficLooper::update(TrafficData *item) {
#ifndef NKR_NO_GRPC
        bool rpcOK;
        auto uplink = NekoRay::rpc::defaultClient->QueryStats(&rpcOK, item->tag, "uplink");
        auto downlink = NekoRay::rpc::defaultClient->QueryStats(&rpcOK, item->tag, "downlink");

        item->downlink += downlink;
        item->uplink += uplink;

        //?
        item->downlink_rate = downlink;
        item->uplink_rate = uplink;
#endif
    }

    [[noreturn]] void TrafficLooper::loop() {
        while (true) {
            if (dataStore->traffic_loop_interval < 500 || dataStore->traffic_loop_interval > 2000)
                dataStore->traffic_loop_interval = 500;
            QThread::msleep(dataStore->traffic_loop_interval);
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
            loop_mutex.unlock();

            // post to UI
            runOnUiThread([=] {
                auto m = GetMainWindow();
                m->refresh_status(
                        QObject::tr("Proxy: %1\nDirect: %2").arg(proxy->DisplaySpeed(), bypass->DisplaySpeed()));
                for (const auto &item: items) {
                    if (item->id < 0) continue;
                    m->refresh_proxy_list(item->id);
                }
            });
        }
    }
}
