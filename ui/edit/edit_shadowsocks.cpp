#include "edit_shadowsocks.h"
#include "ui_edit_shadowsocks.h"

#include "fmt/ShadowSocksBean.hpp"

EditShadowSocks::EditShadowSocks(QWidget *parent) : QWidget(parent),
                                                    ui(new Ui::EditShadowSocks) {
    ui->setupUi(this);
    if (IS_NEKO_BOX) {
        ui->method->addItems({"2022-blake3-aes-128-gcm", "2022-blake3-aes-256-gcm", "2022-blake3-chacha20-poly1305"});
    } else {
        ui->uot->hide();
    }
}

EditShadowSocks::~EditShadowSocks() {
    delete ui;
}

void EditShadowSocks::onStart(std::shared_ptr<NekoGui::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->ShadowSocksBean();

    ui->method->setCurrentText(bean->method);
    ui->uot->setChecked(bean->uot);
    ui->password->setText(bean->password);
    auto ssPlugin = bean->plugin.split(";");
    if (!ssPlugin.empty()) {
        ui->plugin->setCurrentText(ssPlugin[0]);
        ui->plugin_opts->setText(SubStrAfter(bean->plugin, ";"));
    }
}

bool EditShadowSocks::onEnd() {
    auto bean = this->ent->ShadowSocksBean();

    bean->method = ui->method->currentText();
    bean->password = ui->password->text();
    bean->uot = ui->uot->isChecked();
    bean->plugin = ui->plugin->currentText();
    if (!bean->plugin.isEmpty()) {
        bean->plugin += ";" + ui->plugin_opts->text();
    }

    return true;
}
