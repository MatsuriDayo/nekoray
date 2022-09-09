#include "TrayIcon.hpp"

#include <QPainter>

QIcon TrayIcon::GetIcon(TrayIcon::TrayIconStatus status) {
    auto icon = QIcon::fromTheme("nekoray");
    auto pixmap = QPixmap("../nekoray.png");
    if (!pixmap.isNull()) icon = QIcon(pixmap);
    pixmap = QPixmap("./nekoray.png");
    if (!pixmap.isNull()) icon = QIcon(pixmap);

    if (status == TrayIconStatus::NONE) return icon;

    pixmap = icon.pixmap(icon.availableSizes().first());
    auto p = QPainter(&pixmap);

    auto side = pixmap.width();
    auto radius = side * 0.4;
    auto d = side * 0.3;
    auto margin = side * 0.1;

    if (status == TrayIconStatus::RUNNING) {
        p.setBrush(QBrush(Qt::darkGreen));
    } else if (status == TrayIconStatus::SYSTEM_PROXY) {
        p.setBrush(QBrush(Qt::blue));
    } else if (status == TrayIconStatus::VPN) {
        p.setBrush(QBrush(Qt::red));
    }
    p.drawRoundedRect(
            QRect(side - d - margin,
                  side - d - margin,
                  d,
                  d),
            radius,
            radius);
    p.end();

    return pixmap;
}
