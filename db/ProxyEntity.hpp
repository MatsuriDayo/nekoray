#pragma once

#include "main/NekoRay.hpp"
#include "TrafficData.hpp"
#include "fmt/AbstractBean.hpp"

namespace NekoRay {
    namespace fmt {
        class SocksHttpBean;

        class ShadowSocksBean;

        class VMessBean;

        class TrojanVLESSBean;

        class NaiveBean;

        class CustomBean;

        class ChainBean;
    };

    class ProxyEntity : public JsonStore {
    public:
        QString type;

        int id = -1;
        int gid = 0;
        QSharedPointer<fmt::AbstractBean> bean;
        QSharedPointer<traffic::TrafficData> traffic_data = QSharedPointer<traffic::TrafficData>(
                new traffic::TrafficData(""));

        // Cache
        int latency = 0;
        QString full_test_report;

        ProxyEntity(fmt::AbstractBean *bean, QString _type);

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
