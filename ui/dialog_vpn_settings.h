#ifndef NEKORAY_DIALOG_VPN_SETTINGS_H
#define NEKORAY_DIALOG_VPN_SETTINGS_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
    class DialogVPNSettings;
}
QT_END_NAMESPACE

class DialogVPNSettings : public QDialog {
    Q_OBJECT

public:
    explicit DialogVPNSettings(QWidget *parent = nullptr);

    ~DialogVPNSettings() override;

private:
    Ui::DialogVPNSettings *ui;

public slots:

    void accept() override;

    void on_troubleshooting_clicked();
};

#endif // NEKORAY_DIALOG_VPN_SETTINGS_H
