#include "dialog_vpn_settings.h"
#include "ui_dialog_vpn_settings.h"

#include "main/GuiUtils.hpp"
#include "main/NekoRay.hpp"
#include "ui/mainwindow_interface.h"

#include <QMessageBox>

DialogVPNSettings::DialogVPNSettings(QWidget *parent) : QDialog(parent), ui(new Ui::DialogVPNSettings) {
    ui->setupUi(this);
    ADD_ASTERISK(this);

    ui->fake_dns->setVisible(!IS_NEKO_BOX);
    ui->fake_dns->setChecked(NekoRay::dataStore->fake_dns);
    //
    ui->vpn_implementation->setCurrentIndex(NekoRay::dataStore->vpn_implementation);
    ui->vpn_mtu->setCurrentText(Int2String(NekoRay::dataStore->vpn_mtu));
    ui->vpn_ipv6->setChecked(NekoRay::dataStore->vpn_ipv6);
    ui->hide_console->setChecked(NekoRay::dataStore->vpn_hide_console);
#ifndef Q_OS_WIN
    ui->hide_console->setVisible(false);
#endif
    ui->strict_route->setChecked(NekoRay::dataStore->vpn_strict_route);
    ui->single_core->setVisible(IS_NEKO_BOX);
    ui->single_core->setChecked(NekoRay::dataStore->vpn_internal_tun);
    //
    D_LOAD_STRING(vpn_rule_cidr)
    D_LOAD_STRING(vpn_rule_process)
    //
    connect(ui->whitelist_mode, &QCheckBox::stateChanged, this, [=](int state) {
        if (state == Qt::Checked) {
            ui->gb_cidr->setTitle(tr("Proxy CIDR"));
            ui->gb_process_name->setTitle(tr("Proxy Process Name"));
        } else {
            ui->gb_cidr->setTitle(tr("Bypass CIDR"));
            ui->gb_process_name->setTitle(tr("Bypass Process Name"));
        }
    });
    ui->whitelist_mode->setChecked(NekoRay::dataStore->vpn_rule_white);
}

DialogVPNSettings::~DialogVPNSettings() {
    delete ui;
}

void DialogVPNSettings::accept() {
    //
    auto mtu = ui->vpn_mtu->currentText().toInt();
    if (mtu > 10000 || mtu < 1000) mtu = 9000;
    NekoRay::dataStore->vpn_implementation = ui->vpn_implementation->currentIndex();
    NekoRay::dataStore->fake_dns = ui->fake_dns->isChecked();
    NekoRay::dataStore->vpn_mtu = mtu;
    NekoRay::dataStore->vpn_ipv6 = ui->vpn_ipv6->isChecked();
    NekoRay::dataStore->vpn_hide_console = ui->hide_console->isChecked();
    NekoRay::dataStore->vpn_strict_route = ui->strict_route->isChecked();
    NekoRay::dataStore->vpn_rule_white = ui->whitelist_mode->isChecked();
    NekoRay::dataStore->vpn_internal_tun = ui->single_core->isChecked();
    //
    D_SAVE_STRING_QTEXTEDIT(vpn_rule_cidr)
    D_SAVE_STRING_QTEXTEDIT(vpn_rule_process)
    //
    MW_dialog_message("", "UpdateDataStore,VPNChanged");
    QDialog::accept();
}

void DialogVPNSettings::on_troubleshooting_clicked() {
    auto r = QMessageBox::information(this, tr("Troubleshooting"),
                                      tr("If you have trouble starting VPN, you can force reset nekobox_core process here.\n\n"
                                         "If still not working, see documentation for more information.\n"
                                         "https://matsuridayo.github.io/n-configuration/#vpn-tun"),
                                      tr("Reset"), tr("Cancel"), "",
                                      1, 1);
    if (r == 0) {
        GetMainWindow()->StopVPNProcess(true);
    }
}
