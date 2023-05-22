#pragma once

#include "main/NekoGui.hpp"

namespace NekoGui_traffic {
    class TrafficData : public JsonStore {
    public:
        int id = -1; // ent id
        std::string tag;

        long long downlink = 0;
        long long uplink = 0;
        long long downlink_rate = 0;
        long long uplink_rate = 0;

        long long last_update;

        explicit TrafficData(std::string tag) {
            this->tag = std::move(tag);
            _add(new configItem("dl", &downlink, itemType::integer64));
            _add(new configItem("ul", &uplink, itemType::integer64));
        };

        void Reset() {
            downlink = 0;
            uplink = 0;
            downlink_rate = 0;
            uplink_rate = 0;
        }

        [[nodiscard]] QString DisplaySpeed() const {
            return UNICODE_LRO + QString("%1↑ %2↓").arg(ReadableSize(uplink_rate), ReadableSize(downlink_rate));
        }

        [[nodiscard]] QString DisplayTraffic() const {
            if (downlink + uplink == 0) return "";
            return UNICODE_LRO + QString("%1↑ %2↓").arg(ReadableSize(uplink), ReadableSize(downlink));
        }
    };
} // namespace NekoGui_traffic
