#pragma once

#include <utility>

#include "NekoRay.hpp"

namespace NekoRay::traffic {
    class TrafficData : public JsonStore {
    public:
        int id = -1; // ent id
        std::string tag;

        long long downlink = 0;
        long long uplink = 0;
        long long downlink_rate = 0;
        long long uplink_rate = 0;

        explicit TrafficData(std::string tag) {
            this->tag = std::move(tag);
            _add(new configItem("downlink", &downlink, itemType::integer64));
            _add(new configItem("uplink", &uplink, itemType::integer64));
        };

        void Reset() {
            downlink = 0;
            uplink = 0;
            downlink_rate = 0;
            uplink_rate = 0;
        }

        [[nodiscard]] QString DisplaySpeed() const {
            return QString("%1↑ %2↓").arg(ReadableSize(uplink_rate), ReadableSize(downlink_rate));
        }

        [[nodiscard]] QString DisplayTraffic() const {
            if (downlink + uplink == 0) return "";
            return QString("%1↑ %2↓").arg(ReadableSize(uplink), ReadableSize(downlink));
        }

        void Add(TrafficData *other) {
            downlink += other->downlink;
            uplink += other->uplink;
            downlink_rate += other->downlink_rate;
            uplink_rate += other->uplink_rate;
        }
    };
}
