#include "ui_dialog_basic_settings.h"

#include "ui/mainwindow_message.h"
#include "ui/dialog_basic_settings.h"

#include "NekoRay.hpp"

DialogBasicSettings::DialogBasicSettings(QWidget *parent)
        : QDialog(parent), ui(new Ui::DialogBasicSettings) {
    ui->setupUi(this);

    ui->socks_port->setValidator(QRegExpValidator_Number, this));
    ui->http_port->setValidator(QRegExpValidator_Number, this));

    ui->socks_ip->setText(NekoRay::dataStore->inbound_address);
    ui->socks_port->setText(Int2String(NekoRay::dataStore->inbound_socks_port));
    ui->log_level->setCurrentText(NekoRay::dataStore->log_level);
    ui->user_agent->setText(NekoRay::dataStore->user_agent);

    auto http_port = NekoRay::dataStore->inbound_http_port;
    if (http_port > 0) {
        ui->http_enable->setChecked(true);
    } else {
        ui->http_enable->setChecked(false);
        http_port = -http_port;
    }
    ui->http_port->setText(Int2String(http_port));

    auto mux_cool = NekoRay::dataStore->mux_cool;
    if (mux_cool > 0) {
        ui->mux_cool_enable->setChecked(true);
    } else {
        ui->mux_cool_enable->setChecked(false);
        mux_cool = -mux_cool;
    }
    ui->mux_cool->setText(Int2String(mux_cool));

}

DialogBasicSettings::~DialogBasicSettings() {
    delete ui;
}

void DialogBasicSettings::accept() {
    NekoRay::dataStore->inbound_address = ui->socks_ip->text();
    NekoRay::dataStore->inbound_socks_port = ui->socks_port->text().toInt();
    NekoRay::dataStore->log_level = ui->log_level->currentText();
    NekoRay::dataStore->user_agent = ui->user_agent->text();

    if (ui->http_enable->isChecked()) {
        NekoRay::dataStore->inbound_http_port = ui->http_port->text().toInt();
    } else {
        NekoRay::dataStore->inbound_http_port = -ui->http_port->text().toInt();
    }

    if (ui->mux_cool_enable->isChecked()) {
        NekoRay::dataStore->mux_cool = ui->mux_cool->text().toInt();
    } else {
        NekoRay::dataStore->mux_cool = -ui->mux_cool->text().toInt();
    }

    emit GetMainWindow()->dialog_message(Dialog_DialogBasicSettings, "SaveDataStore");
    QDialog::accept();
}
