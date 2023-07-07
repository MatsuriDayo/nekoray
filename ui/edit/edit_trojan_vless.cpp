#include "edit_trojan_vless.h"
#include "ui_edit_trojan_vless.h"

#include "fmt/TrojanVLESSBean.hpp"
#include "fmt/Preset.hpp"

EditTrojanVLESS::EditTrojanVLESS(QWidget *parent) : QWidget(parent), ui(new Ui::EditTrojanVLESS) {
    ui->setupUi(this);
}

EditTrojanVLESS::~EditTrojanVLESS() {
    delete ui;
}

void EditTrojanVLESS::onStart(std::shared_ptr<NekoGui::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->TrojanVLESSBean();
    if (bean->proxy_type == NekoGui_fmt::TrojanVLESSBean::proxy_VLESS) {
        ui->label->setText("UUID");
    }
    if (bean->proxy_type != NekoGui_fmt::TrojanVLESSBean::proxy_VLESS) {
        ui->flow->hide();
        ui->flow_l->hide();
    }
    ui->password->setText(bean->password);
    ui->flow->addItems(IS_NEKO_BOX ? Preset::SingBox::Flows : Preset::Xray::Flows);
    ui->flow->setCurrentText(bean->flow);
}

bool EditTrojanVLESS::onEnd() {
    auto bean = this->ent->TrojanVLESSBean();
    bean->password = ui->password->text();
    bean->flow = ui->flow->currentText();
    return true;
}
