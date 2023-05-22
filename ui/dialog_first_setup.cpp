#include "dialog_first_setup.h"
#include "ui_dialog_first_setup.h"

#include "main/NekoGui.hpp"

DialogFirstSetup::DialogFirstSetup(QWidget *parent) : QDialog(parent), ui(new Ui::DialogFirstSetup) {
    ui->setupUi(this);
}

DialogFirstSetup::~DialogFirstSetup() {
    delete ui;
}

void DialogFirstSetup::onButtonClicked() {
    auto s = sender();
    if (s == ui->v2ray) {
        NekoGui::coreType = NekoGui::CoreType::V2RAY;
    } else if (s == ui->singbox) {
        NekoGui::coreType = NekoGui::CoreType::SING_BOX;
    }
    done(0);
}
