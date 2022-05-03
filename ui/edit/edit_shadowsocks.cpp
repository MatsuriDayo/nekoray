#include "edit_shadowsocks.h"
#include "ui_edit_shadowsocks.h"

EditShadowSocks::EditShadowSocks(QWidget *parent) : QWidget(parent),
                                                    ui(new Ui::EditShadowSocks) {
    ui->setupUi(this);
}

EditShadowSocks::~EditShadowSocks() {
    delete ui;
}

void EditShadowSocks::onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) {
    // TODO ssr
    this->ent = _ent;
    auto bean = this->ent->ShadowSocksBean();

    ui->method->setCurrentText(bean->method);
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
    bean->plugin = ui->plugin->currentText();
    if (!bean->plugin.isEmpty()) {
        bean->plugin += ";" + ui->plugin_opts->text();
    }

    return true;
}