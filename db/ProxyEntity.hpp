#pragma once

#include <utility>

#include "main/NekoRay.hpp"
#include "TrafficData.hpp"
#include "fmt/AbstractBean.hpp"
#include "fmt/SocksBean.hpp"
#include "fmt/ShadowSocksBean.hpp"
#include "fmt/ChainBean.hpp"
#include "fmt/VMessBean.hpp"
#include "fmt/TrojanBean.hpp"
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

        ProxyEntity(fmt::AbstractBean *bean, QString type);

        [[nodiscard]] QString DisplayLatency() const;

        [[nodiscard]] fmt::ChainBean *ChainBean() const {
            return (fmt::ChainBean *) bean.get();
        };

        [[nodiscard]] fmt::SocksBean *SocksBean() const {
            return (fmt::SocksBean *) bean.get();
        };

        [[nodiscard]] fmt::ShadowSocksBean *ShadowSocksBean() const {
            return (fmt::ShadowSocksBean *) bean.get();
        };

        [[nodiscard]] fmt::VMessBean *VMessBean() const {
            return (fmt::VMessBean *) bean.get();
        };

        [[nodiscard]] fmt::TrojanBean *TrojanBean() const {
            return (fmt::TrojanBean *) bean.get();
        };

        [[nodiscard]] fmt::NaiveBean *NaiveBean() const {
            return (fmt::NaiveBean *) bean.get();
        };

        [[nodiscard]] fmt::CustomBean *CustomBean() const {
            return (fmt::CustomBean *) bean.get();
        };

    };
}
