#pragma once

#include <utility>

#include "main/NekoRay.hpp"
#include "TrafficData.hpp"
#include "fmt/AbstractBean.hpp"
#include "fmt/SocksHttpBean.hpp"
#include "fmt/ShadowSocksBean.hpp"
#include "fmt/ChainBean.hpp"
#include "fmt/VMessBean.hpp"
#include "fmt/TrojanVLESSBean.hpp"
#include "fmt/NaiveBean.hpp"
#include "fmt/CustomBean.hpp"

namespace NekoRay {
    class ProxyEntity : public JsonStore {
    public:
        QString type = "";

        int id = -1;
        int gid = 0;
        QSharedPointer<fmt::AbstractBean> bean;
        QSharedPointer<traffic::TrafficData> traffic_data = QSharedPointer<traffic::TrafficData>(
                new traffic::TrafficData(""));

        // Cache
        int latency = 0;
        QString full_test_report = "";

        ProxyEntity(fmt::AbstractBean *bean, QString type);

        [[nodiscard]] QString DisplayLatency() const;

        [[nodiscard]] fmt::ChainBean *ChainBean() const {
            return (fmt::ChainBean *) bean.get();
        };

        [[nodiscard]] fmt::SocksHttpBean *SocksHTTPBean() const {
            return (fmt::SocksHttpBean *) bean.get();
        };

        [[nodiscard]] fmt::ShadowSocksBean *ShadowSocksBean() const {
            return (fmt::ShadowSocksBean *) bean.get();
        };

        [[nodiscard]] fmt::VMessBean *VMessBean() const {
            return (fmt::VMessBean *) bean.get();
        };

        [[nodiscard]] fmt::TrojanVLESSBean *TrojanVLESSBean() const {
            return (fmt::TrojanVLESSBean *) bean.get();
        };

        [[nodiscard]] fmt::NaiveBean *NaiveBean() const {
            return (fmt::NaiveBean *) bean.get();
        };

        [[nodiscard]] fmt::CustomBean *CustomBean() const {
            return (fmt::CustomBean *) bean.get();
        };

    };
}
