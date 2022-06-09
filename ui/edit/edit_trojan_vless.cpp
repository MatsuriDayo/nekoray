#include "edit_trojan_vless.h"
#include "ui_edit_trojan_vless.h"

EditTrojanVLESS::EditTrojanVLESS(QWidget *parent) :
        QWidget(parent), ui(new Ui::EditTrojanVLESS) {
    ui->setupUi(this);
}

EditTrojanVLESS::~EditTrojanVLESS() {
    delete ui;
}

void EditTrojanVLESS::onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->TrojanVLESSBean();
    if (bean->proxy_type == NekoRay::fmt::TrojanVLESSBean::proxy_VLESS) ui->label->setText("UUID");
    ui->password->setText(bean->password);
}

bool EditTrojanVLESS::onEnd() {
    auto bean = this->ent->TrojanVLESSBean();
    bean->password = ui->password->text();
    return true;
}
