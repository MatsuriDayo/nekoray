#include "edit_trojan.h"
#include "ui_edit_trojan.h"

EditTrojan::EditTrojan(QWidget *parent) :
        QWidget(parent), ui(new Ui::EditTrojan) {
    ui->setupUi(this);
}

EditTrojan::~EditTrojan() {
    delete ui;
}

void EditTrojan::onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->TrojanBean();
    ui->password->setText(bean->password);
}

bool EditTrojan::onEnd() {
    auto bean = this->ent->TrojanBean();
    bean->password = ui->password->text();
    return true;
}
