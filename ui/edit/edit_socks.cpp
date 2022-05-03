#include "edit_socks.h"
#include "ui_edit_socks.h"

EditSocks::EditSocks(QWidget *parent) : QWidget(parent),
                                        ui(new Ui::EditSocks) {
    ui->setupUi(this);
}

EditSocks::~EditSocks() {
    delete ui;
}

void EditSocks::onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->SocksBean();

    if (bean->socksVersion == 4) {
        ui->version->setCurrentIndex(1);
    } else {
        ui->version->setCurrentIndex(0);
    }

    ui->username->setText(bean->username);
    ui->password->setText(bean->password);
}

bool EditSocks::onEnd() {
    auto bean = this->ent->SocksBean();

    if (ui->version->currentIndex() == 1) {
        bean->socksVersion = 4;
    } else {
        bean->socksVersion = 5;
    }

    bean->username = ui->username->text();
    bean->password = ui->password->text();

    return true;
}