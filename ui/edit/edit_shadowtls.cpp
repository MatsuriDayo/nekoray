#include "edit_shadowtls.h"
#include "ui_edit_shadowtls.h"

#include "fmt/ShadowTLSBean.hpp"

EditShadowTLS::EditShadowTLS(QWidget *parent) : QWidget(parent), ui(new Ui::EditShadowTLS) {
    ui->setupUi(this);
}

EditShadowTLS::~EditShadowTLS() {
    delete ui;
}

void EditShadowTLS::onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->ShadowTLSBean();

    ui->password->setText(bean->password);
    ui->shadowTLSVersion->setCurrentText(QString::number(bean->shadowTLSVersion));
    // ui->shadowTLSVersion->setCurrentText(Int2String(bean->shadowTLSVersion));
}

bool EditShadowTLS::onEnd() {
    auto bean = this->ent->ShadowTLSBean();

    bean->password = ui->password->text();
    bean->shadowTLSVersion = ui->shadowTLSVersion->currentText().toInt();

    return true;
}
