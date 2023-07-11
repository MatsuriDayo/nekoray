#include "edit_shadowsocks.h"
#include "ui_edit_shadowsocks.h"

#include "fmt/ShadowSocksBean.hpp"
#include "fmt/Preset.hpp"

EditShadowSocks::EditShadowSocks(QWidget *parent) : QWidget(parent),
                                                    ui(new Ui::EditShadowSocks) {
    ui->setupUi(this);
    ui->method->addItems(IS_NEKO_BOX ? Preset::SingBox::ShadowsocksMethods : Preset::Xray::ShadowsocksMethods);
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
