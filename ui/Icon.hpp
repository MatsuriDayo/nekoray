#pragma once

#include <QIcon>

namespace Icon {

    enum TrayIconStatus {
        NONE,
        RUNNING,
        SYSTEM_PROXY,
        VPN,
    };

    QIcon GetTrayIcon(TrayIconStatus status);

    QIcon GetMaterialIcon(const QString &name);

} // namespace Icon
