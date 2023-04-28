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

        class HysteriaBean;

        class CustomBean;

        class ChainBean;
    }; // namespace fmt

    class ProxyEntity : public JsonStore {
    public:
        QString type;

        int id = -1;
        int gid = 0;
        int latency = 0;
        QSharedPointer<fmt::AbstractBean> bean;
        QSharedPointer<traffic::TrafficData> traffic_data = QSharedPointer<traffic::TrafficData>(new traffic::TrafficData(""));

        QString full_test_report;

        ProxyEntity(fmt::AbstractBean *bean, const QString &type_);

        [[nodiscard]] QString DisplayLatency() const;

        [[nodiscard]] QColor DisplayLatencyColor() const;

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

        [[nodiscard]] fmt::HysteriaBean *HysteriaBean() const {
            return (fmt::HysteriaBean *) bean.get();
        };

        [[nodiscard]] fmt::CustomBean *CustomBean() const {
            return (fmt::CustomBean *) bean.get();
        };
    };
} // namespace NekoRay
