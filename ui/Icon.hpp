#pragma once

#include <QPixmap>

namespace Icon {

    enum TrayIconStatus {
        NONE,
        RUNNING,
        SYSTEM_PROXY,
        VPN,
    };

    QPixmap GetTrayIcon(TrayIconStatus status);

    QPixmap GetMaterialIcon(const QString &name);

} // namespace Icon
