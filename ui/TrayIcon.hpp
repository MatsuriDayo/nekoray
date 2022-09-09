#pragma once

#include <QIcon>

namespace TrayIcon {

    enum TrayIconStatus {
        NONE,
        RUNNING,
        SYSTEM_PROXY,
        VPN,
    };

    QIcon GetIcon(TrayIconStatus status);

}
