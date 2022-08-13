#include "gen_hysteria.h"
#include "ui_gen_hysteria.h"

#include "fmt/CustomBean.hpp"
#include "fmt/Preset.hpp"

GenHysteria::GenHysteria(const QSharedPointer<NekoRay::ProxyEntity> &ent, QWidget *parent) :
        QWidget(parent), ui(new Ui::GenHysteria) {
    ui->setupUi(this);
    this->ent = ent;
}

GenHysteria::~GenHysteria() {
    delete ui;
}

void GenHysteria::on_gen_clicked() {
    auto result = QString2QJsonObject(Preset::Hysteria::config);
    result["obfs"] = ui->obfs_password->text();
    result["insecure"] = ui->allow_insecure->isChecked();
    result["protocol"] = ui->protocol->currentText();
    result["up_mbps"] = ui->up_mbps->text().toInt();
    result["down_mbps"] = ui->down_mbps->text().toInt();
    if (!ui->auth_string->text().isEmpty()) {
        result["auth_str"] = ui->auth_string->text();
    }
    if (!ui->alpn->text().isEmpty()) {
        result["alpn"] = ui->alpn->text();
    }
    if (!ui->sni->text().isEmpty()) {
        result["server_name"] = ui->sni->text();
    }
    if (!ui->cert_path->text().isEmpty()) {
        result["ca"] = ui->cert_path->text();
    }
    emit config_generated(QJsonObject2QString(result, false));
}
