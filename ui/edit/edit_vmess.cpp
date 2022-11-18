#include "edit_vmess.h"
#include "ui_edit_vmess.h"

#include "fmt/VMessBean.hpp"

EditVMess::EditVMess(QWidget *parent) : QWidget(parent), ui(new Ui::EditVMess) {
    ui->setupUi(this);
}

EditVMess::~EditVMess() {
    delete ui;
}

void EditVMess::onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->VMessBean();

    ui->uuid->setText(bean->uuid);
    ui->aid->setText(Int2String(bean->aid));
    ui->security->setCurrentText(bean->security);
}

bool EditVMess::onEnd() {
    auto bean = this->ent->VMessBean();

    bean->uuid = ui->uuid->text();
    bean->aid = ui->aid->text().toInt();
    bean->security = ui->security->currentText();

    return true;
}
