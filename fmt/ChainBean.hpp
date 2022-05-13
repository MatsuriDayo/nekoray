#pragma once

#include "NekoRay.hpp"

namespace NekoRay::fmt {
    class ChainBean : public AbstractBean {
    public:
        QList<int> list; // in to out

        ChainBean() : AbstractBean(0) {
            _add(new configItem("list", &list, itemType::integerList));
        };

        QString DisplayType() override { return "Chain Proxy"; };

        QString DisplayAddress() override { return ""; };
    };
}
