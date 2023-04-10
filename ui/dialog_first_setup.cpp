#include "dialog_first_setup.h"
#include "ui_dialog_first_setup.h"

#include "main/NekoRay.hpp"

DialogFirstSetup::DialogFirstSetup(QWidget *parent) : QDialog(parent), ui(new Ui::DialogFirstSetup) {
    ui->setupUi(this);
}

DialogFirstSetup::~DialogFirstSetup() {
    delete ui;
}

void DialogFirstSetup::onButtonClicked() {
    auto s = sender();
    if (s == ui->v2ray) {
        done(NekoRay::CoreType::V2RAY);
    } else if (s == ui->singbox) {
        done(NekoRay::CoreType::SING_BOX);
    } else {
        done(-1);
    }
}
