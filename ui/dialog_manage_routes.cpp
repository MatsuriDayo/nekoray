#include <QInputDialog>
#include "dialog_manage_routes.h"
#include "ui_dialog_manage_routes.h"

#include "ui/mainwindow_message.h"

DialogManageRoutes::DialogManageRoutes(QWidget *parent) :
        QDialog(parent), ui(new Ui::DialogManageRoutes) {
    ui->setupUi(this);

    ui->sniffing_mode->setCurrentIndex(NekoRay::dataStore->sniffing_mode);
    ui->outbound_domain_strategy->setCurrentText(NekoRay::dataStore->outbound_domain_strategy);
    ui->domainMatcherCombo->setCurrentIndex(NekoRay::dataStore->domain_matcher);
    ui->domainStrategyCombo->setCurrentText(NekoRay::dataStore->domain_strategy);
    ui->dns_routing->setChecked(NekoRay::dataStore->dns_routing);
    ui->dns_remote->setText(NekoRay::dataStore->remote_dns);
    ui->dns_direct->setText(NekoRay::dataStore->direct_dns);

    builtInSchemesMenu = new QMenu(this);
    builtInSchemesMenu->addActions(this->getBuiltInSchemes());
    ui->preset->setMenu(builtInSchemesMenu);

    //
    directDomainTxt = new AutoCompleteTextEdit("geosite", {}, this);
    proxyDomainTxt = new AutoCompleteTextEdit("geosite", {}, this);
    blockDomainTxt = new AutoCompleteTextEdit("geosite", {}, this);
    //
    directIPTxt = new AutoCompleteTextEdit("geoip", {}, this);
    proxyIPTxt = new AutoCompleteTextEdit("geoip", {}, this);
    blockIPTxt = new AutoCompleteTextEdit("geoip", {}, this);
    //
    ui->directTxtLayout->addWidget(directDomainTxt, 0, 0);
    ui->proxyTxtLayout->addWidget(proxyDomainTxt, 0, 0);
    ui->blockTxtLayout->addWidget(blockDomainTxt, 0, 0);
    //
    ui->directIPLayout->addWidget(directIPTxt, 0, 0);
    ui->proxyIPLayout->addWidget(proxyIPTxt, 0, 0);
    ui->blockIPLayout->addWidget(blockIPTxt, 0, 0);

    directIPTxt->setPlainText(NekoRay::dataStore->routing->direct_ip);
    directDomainTxt->setPlainText(NekoRay::dataStore->routing->direct_domain);
    proxyIPTxt->setPlainText(NekoRay::dataStore->routing->proxy_ip);
    proxyDomainTxt->setPlainText(NekoRay::dataStore->routing->proxy_domain);
    blockIPTxt->setPlainText(NekoRay::dataStore->routing->block_ip);
    blockDomainTxt->setPlainText(NekoRay::dataStore->routing->block_domain);
}

DialogManageRoutes::~DialogManageRoutes() {
    delete ui;
}

void DialogManageRoutes::accept() {
    NekoRay::dataStore->sniffing_mode = ui->sniffing_mode->currentIndex();
    NekoRay::dataStore->domain_matcher = ui->domainMatcherCombo->currentIndex();
    NekoRay::dataStore->domain_strategy = ui->domainStrategyCombo->currentText();
    NekoRay::dataStore->outbound_domain_strategy = ui->outbound_domain_strategy->currentText();
    NekoRay::dataStore->dns_routing = ui->dns_routing->isChecked();
    NekoRay::dataStore->remote_dns = ui->dns_remote->text();
    NekoRay::dataStore->direct_dns = ui->dns_direct->text();

    NekoRay::dataStore->routing->direct_ip = directIPTxt->toPlainText();
    NekoRay::dataStore->routing->direct_domain = directDomainTxt->toPlainText();
    NekoRay::dataStore->routing->proxy_ip = proxyIPTxt->toPlainText();
    NekoRay::dataStore->routing->proxy_domain = proxyDomainTxt->toPlainText();
    NekoRay::dataStore->routing->block_ip = blockIPTxt->toPlainText();
    NekoRay::dataStore->routing->block_domain = blockDomainTxt->toPlainText();

    emit GetMainWindow()->dialog_message(Dialog_DialogManageRoutes, "SaveDataStore");
    QDialog::accept();
}

// built in settings

QList<QAction *> DialogManageRoutes::getBuiltInSchemes() {
    QList<QAction *> list;
    list.append(this->schemeToAction(tr("Bypass LAN and China"), routing_cn_lan));
    list.append(this->schemeToAction(tr("Global"), routing_global));
    return list;
}

QAction *DialogManageRoutes::schemeToAction(const QString &name, const NekoRay::Routing &scheme) {
    auto *action = new QAction(this);
    action->setText(name);
    connect(action, &QAction::triggered, [this, &scheme] { this->SetRouteConfig(scheme); });
    return action;
}

void DialogManageRoutes::SetRouteConfig(const NekoRay::Routing &conf) {
    //
    directDomainTxt->setPlainText(conf.direct_domain);
    proxyDomainTxt->setPlainText(conf.proxy_domain);
    blockDomainTxt->setPlainText(conf.block_domain);
    //
    blockIPTxt->setPlainText(conf.block_ip);
    directIPTxt->setPlainText(conf.direct_ip);
    proxyIPTxt->setPlainText(conf.proxy_ip);
}
