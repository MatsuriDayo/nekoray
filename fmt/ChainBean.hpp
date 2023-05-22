#pragma once

#include "main/NekoGui.hpp"

namespace NekoGui_fmt {
    class ChainBean : public AbstractBean {
    public:
        QList<int> list; // in to out

        ChainBean() : AbstractBean(0) {
            _add(new configItem("list", &list, itemType::integerList));
        };

        QString DisplayType() override { return QObject::tr("Chain Proxy"); };

        QString DisplayAddress() override { return ""; };
    };
} // namespace NekoGui_fmt
