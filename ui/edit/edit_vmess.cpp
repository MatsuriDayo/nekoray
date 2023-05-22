#include "edit_vmess.h"
#include "ui_edit_vmess.h"

#include "fmt/VMessBean.hpp"

#include <QUuid>

EditVMess::EditVMess(QWidget *parent) : QWidget(parent), ui(new Ui::EditVMess) {
    ui->setupUi(this);
    connect(ui->uuidgen, &QPushButton::clicked, this, [=] { ui->uuid->setText(QUuid::createUuid().toString().remove("{").remove("}")); });
}

EditVMess::~EditVMess() {
    delete ui;
}

void EditVMess::onStart(std::shared_ptr<NekoGui::ProxyEntity> _ent) {
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
